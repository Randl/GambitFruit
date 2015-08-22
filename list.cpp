// list.cpp

// includes

#include "board.h"
#include "list.h"
#include "move.h"

// constants

static constexpr bool UseStrict = true;

// functions

// list_t::is_ok()

bool list_t::is_ok() const {

	//if (list == nullptr) return false;
	if (size < 0 || size >= ListSize) return false;

	return true;
}

// list_t::remove()

void list_t::remove(uint_fast16_t pos) {

	ASSERT(is_ok());
	ASSERT(pos >= 0 && pos < size);

	for (int_fast32_t i = pos; i < size - 1; ++i) {
		moves[i].move  = moves[i + 1].move;
		moves[i].value = moves[i + 1].value;
	}

	size--;
}


list_t::list_t(const list_t &src) {
	ASSERT(src.is_ok());

	size = src.size;

	for (int_fast32_t i = 0; i < src.size; ++i) {
		moves[i].move  = src.moves[i].move;
		moves[i].value = src.moves[i].value;
	}
}

list_t::list_t(const list_t *src) {
	ASSERT(src->is_ok());

	size = src->size;

	for (int_fast32_t i = 0; i < src->size; ++i) {
		moves[i].move  = src->moves[i].move;
		moves[i].value = src->moves[i].value;
	}
}

list_t &list_t::operator=(const list_t &src) {
	ASSERT(src.is_ok());

	size = src.size;

	for (int_fast32_t i = 0; i < src.size; ++i) {
		moves[i].move  = src.moves[i].move;
		moves[i].value = src.moves[i].value;
	}

	return *this;
}

// list_t::sort()

void list_t::sort() {

	ASSERT(is_ok());

	// init
	moves[size].value = -32768; // HACK: sentinel

	// insert sort (stable) TODO: better sort?

	for (int_fast16_t i = size - 2; i >= 0; --i) {
		const int_fast32_t move = moves[i].move, value = moves[i].value;
		int_fast16_t       j;
		for (j = i; value < moves[j + 1].value; ++j) {
			moves[j].move  = moves[j + 1].move;
			moves[j].value = moves[j + 1].value;
		}

		ASSERT(j < size);

		moves[j].move  = move;
		moves[j].value = value;
	}

	// debug
	if (DEBUG) {
		for (int_fast32_t i = 0; i < size - 1; ++i) {
			ASSERT(moves[i].value >= moves[i + 1].value);
		}
	}
}

// list_t::contains()

bool list_t::contains(uint_fast16_t move) const {

	ASSERT(is_ok());
	ASSERT(move_is_ok(move));

	for (int_fast32_t i = 0; i < size; ++i)
		if (moves[i].move == move) return true;

	return false;
}

// list_t::note()

void list_t::note() {

	ASSERT(is_ok());

	for (int_fast32_t i = 0; i < size; ++i) {
		const int_fast32_t move = moves[i].move;
		ASSERT(move_is_ok(move));
		moves[i].value = -move_order(move);
	}
}

// list_t::filter()

void list_t::filter(board_t *board, move_test_t test, bool keep) {

	ASSERT(board != nullptr);
	ASSERT(test != nullptr);
	ASSERT(keep == true || keep == false);

	int_fast32_t pos = 0;

	for (int_fast32_t i = 0; i < size; ++i) {

		ASSERT(pos >= 0 && pos <= i);

		const uint_fast16_t move  = moves[i].move;
		const int_fast16_t  value = moves[i].value;

		if ((*test)(move, board) == keep) {
			moves[pos].move  = move;
			moves[pos].value = value;
			++pos;
		}
	}

	ASSERT(pos >= 0 && pos <= size);
	size = pos;

	// debug
	ASSERT(is_ok());
}
// end of list.cpp
