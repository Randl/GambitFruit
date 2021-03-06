// trans.cpp

// includes

#include "hash.h"
#include "move.h"
#include "option.h"
#include "protocol.h"
#include "trans.h"
#include "value.h"



// constants

static constexpr bool UseModulo = false;

static constexpr U8 DateSize = 16;

static constexpr U8 ClusterSize = 4;

static constexpr S16 DepthNone = -128;

// types

struct entry_t {
    U32 lock;
    U16 move;
    S16 min_value;
    S16 max_value;
    S8 depth;
    U8 date;
    S8 move_depth;
    U8 flags;
    S8 min_depth;
    S8 max_depth;
};

struct trans { // HACK: typedef'ed in trans.h
    S64 read_nb;
    S64 read_hit;
    S64 write_nb;
    S64 write_hit;
    S64 write_collision;
    size_t size;
    size_t mask;
    entry_t *table;
    S32 age[DateSize];
    S32 date;
    U32 used;
};

// variables

class key;

trans_t Trans[1];

// prototypes

static void trans_set_date(trans_t *trans, S32 date);
static S32 trans_age(const trans_t *trans, S32 date);

static entry_t *trans_entry(trans_t *trans, U64 key);

static bool entry_is_ok(const entry_t *entry);

// functions

// trans_is_ok()

bool trans_is_ok(const trans_t *trans) {

	if (trans == nullptr) return false;

	if (trans->table == nullptr) return false;
	if (trans->size == 0) return false;
	if (trans->mask == 0 || trans->mask >= trans->size) return false;
	if (trans->date >= DateSize) return false;

	for (U8 date = 0; date < DateSize; ++date)
		if (trans->age[date] != trans_age(trans, date)) return false;

	return true;
}

// trans_init()

void trans_init(trans_t *trans) {

	ASSERT(trans != nullptr);
	ASSERT(sizeof(entry_t) == 16);

	trans->size = 0;
	trans->mask = 0;
	trans->table = nullptr;

	trans_set_date(trans, 0);

	trans_clear(trans);

	// ASSERT(trans_is_ok(trans));
}

// trans_alloc()

void trans_alloc(trans_t *trans) {

	ASSERT(trans != nullptr);

	// calculate size
	U64 target = option_get_int("Hash");
	target *= 1024 * 1024;

	U64 size;
	for (size = 1; size != 0 && size <= target; size *= 2);

	size /= 2;
	ASSERT(size > 0 && size <= target);

	// allocate table
	size /= sizeof(entry_t);
	ASSERT(size != 0 && (size & (size - 1)) == 0); // power of 2

	trans->size = size + (ClusterSize - 1); // HACK to avoid testing for end of table
	trans->mask = size - 1;

	trans->table = (entry_t *) my_malloc(trans->size * sizeof(entry_t));
//	trans->table = new entry_t[trans->size];

	trans_clear(trans);

	ASSERT(trans_is_ok(trans));
}

// trans_free()

void trans_free(trans_t *trans) {

	ASSERT(trans_is_ok(trans));

	my_free(trans->table);

	trans->table = nullptr;
	trans->size = 0;
	trans->mask = 0;
}

// trans_clear()

void trans_clear(trans_t *trans) {

	ASSERT(trans != nullptr);

	trans_set_date(trans, 0);

	entry_t clear_entry[1];
	clear_entry->lock = 0;
	clear_entry->move = MoveNone;
	clear_entry->depth = DepthNone;
	clear_entry->date = trans->date;
	clear_entry->move_depth = DepthNone;
	clear_entry->flags = 0;
	clear_entry->min_depth = DepthNone;
	clear_entry->max_depth = DepthNone;
	clear_entry->min_value = -ValueInf;
	clear_entry->max_value = +ValueInf;

	ASSERT(entry_is_ok(clear_entry));
	entry_t *entry;
	entry = trans->table;

	for (U32 index = 0; index < trans->size; ++index)
		*entry++ = *clear_entry;
}

// trans_inc_date()

void trans_inc_date(trans_t *trans) {

	ASSERT(trans != nullptr);
	trans_set_date(trans, (trans->date + 1) % DateSize);
}

// trans_set_date()

static void trans_set_date(trans_t *trans, S32 date) {

	ASSERT(trans != nullptr);
	ASSERT(date >= 0 && date < DateSize);

	trans->date = date;

	for (date = 0; date < DateSize; ++date)
		trans->age[date] = trans_age(trans, date);

	trans->used = 0;
	trans->read_nb = 0;
	trans->read_hit = 0;
	trans->write_nb = 0;
	trans->write_hit = 0;
	trans->write_collision = 0;
}

// trans_age()

static S32 trans_age(const trans_t *trans, S32 date) {

	ASSERT(trans != nullptr);
	ASSERT(date >= 0 && date < DateSize);

	S32 age = trans->date - date;
	if (age < 0) age += DateSize;

	ASSERT(age >= 0 && age < DateSize);
	return age;
}

// trans_store()

void trans_store(trans_t *trans, U64 key, U16 move, S32 depth, S32 min_value, S32 max_value) {

	ASSERT(trans_is_ok(trans));
	//ASSERT(move >= 0 && move < 65536); move is uint16
	ASSERT(depth >= -127 && depth <= +127);
	ASSERT(min_value >= -ValueInf && min_value <= +ValueInf);
	ASSERT(max_value >= -ValueInf && max_value <= +ValueInf);
	ASSERT(min_value <= max_value);

	// init
	trans->write_nb++;

	// probe
	entry_t *best_entry = nullptr;
	S32 best_score = -32767;

	entry_t *entry = trans_entry(trans, key);

	for (U8 i = 0; i < ClusterSize; ++i, ++entry) {
		if (entry->lock == KEY_LOCK(key)) {
			// hash hit => update existing entry
			trans->write_hit++;
			if (entry->date != trans->date) trans->used++;
			entry->date = trans->date;

			if (trans_endgame || depth > entry->depth) entry->depth = depth;
			/* if (depth > entry->depth)  entry->depth = depth; // for replacement scheme */
			// if (move != MoveNone /* && depth >= entry->move_depth */) {

			if (move != MoveNone && (trans_endgame || depth >= entry->move_depth)) {
				entry->move_depth = depth;
				entry->move = move;
			}

			// if (min_value > -ValueInf /* && depth >= entry->min_depth */) {
			if (min_value > -ValueInf && (trans_endgame || depth >= entry->min_depth)) {
				entry->min_depth = depth;
				entry->min_value = min_value;
			}

			// if (max_value < +ValueInf /* && depth >= entry->max_depth */) {
			if (max_value < +ValueInf && (trans_endgame || depth >= entry->max_depth)) {
				entry->max_depth = depth;
				entry->max_value = max_value;
			}

			ASSERT(entry_is_ok(entry));
			return;
		}

		// evaluate replacement score

		const S32 score = trans->age[entry->date] * 256 - entry->depth;
		ASSERT(score > -32767);

		if (score > best_score) {
			best_entry = entry;
			best_score = score;
		}
	}

	// "best" entry found

	entry = best_entry;
	ASSERT(entry != nullptr);
	ASSERT(entry->lock != KEY_LOCK(key));

	if (entry->date == trans->date)
		trans->write_collision++;
	else
		trans->used++;

	// store

	ASSERT(entry != nullptr);

	entry->lock = KEY_LOCK(key);
	entry->date = trans->date;
	entry->depth = depth;

	entry->move_depth = (move != MoveNone) ? depth : DepthNone;
	entry->move = move;

	entry->min_depth = (min_value > -ValueInf) ? depth : DepthNone;
	entry->max_depth = (max_value < +ValueInf) ? depth : DepthNone;
	entry->min_value = min_value;
	entry->max_value = max_value;

	ASSERT(entry_is_ok(entry));
}

// trans_retrieve()

bool trans_retrieve(trans_t *trans,
                    U64 key,
                    S32 *move,
                    S32 *min_depth,
                    S32 *max_depth,
                    S32 *min_value,
                    S32 *max_value) {

	ASSERT(trans_is_ok(trans));
	ASSERT(move != nullptr);
	ASSERT(min_depth != nullptr);
	ASSERT(max_depth != nullptr);
	ASSERT(min_value != nullptr);
	ASSERT(max_value != nullptr);

	// init
	trans->read_nb++;

	// probe
	entry_t *entry = trans_entry(trans, key);

	for (U8 i = 0; i < ClusterSize; ++i, ++entry)
		if (entry->lock == KEY_LOCK(key)) {
			// found
			trans->read_hit++;
			if (entry->date != trans->date) entry->date = trans->date;
			*move = entry->move;

			*min_depth = entry->min_depth;
			*max_depth = entry->max_depth;
			*min_value = entry->min_value;
			*max_value = entry->max_value;

			return true;
		}

	// not found
	return false;
}

// trans_stats()

void trans_stats(const trans_t *trans) {

	ASSERT(trans_is_ok(trans));

	double full = double(trans->used) / double(trans->size);
	// double hit = double(trans->read_hit) / double(trans->read_nb);
	// double collision = double(trans->write_collision) / double(trans->write_nb);

	send("info hashfull %.0f", full * 1000.0);
}

// trans_entry()

static entry_t *trans_entry(trans_t *trans, U64 key) {

	ASSERT(trans_is_ok(trans));
	U32 index;
	if (UseModulo)
		index = KEY_INDEX(key) % (trans->mask + 1);
	else
		index = KEY_INDEX(key) & trans->mask;

	ASSERT(index <= trans->mask);

	return &trans->table[index];
}

// entry_is_ok()

static bool entry_is_ok(const entry_t *entry) {

	if (entry == nullptr) return false;

	if (entry->date >= DateSize) return false;

	if (entry->move == MoveNone && entry->move_depth != DepthNone) return false;
	if (entry->move != MoveNone && entry->move_depth == DepthNone) return false;

	if (entry->min_value == -ValueInf && entry->min_depth != DepthNone) return false;
	if (entry->min_value > -ValueInf && entry->min_depth == DepthNone) return false;

	if (entry->max_value == +ValueInf && entry->max_depth != DepthNone) return false;
	if (entry->max_value < +ValueInf && entry->max_depth == DepthNone) return false;

	return true;
}

// end of trans.cpp
