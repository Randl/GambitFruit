// option.cpp

// includes

#include <cstdlib>

#include "option.h"
#include "protocol.h"

// types

struct option_t {
    const char *var;
    bool declare;
    const char *init;
    const char *type;
    const char *extra;
    const char *val;
};

// variables
static option_t Option[] = {

	{"Hash", true, "64", "spin", "min 16 max 2048", nullptr},

	// JAS
	// search X seconds for the best move, equal to "go movetime"
	{"Search Time", true, "0", "spin", "min 0 max 3600", nullptr},
	// search X plies deep, equal to "go depth"
	{"Search Depth", true, "0", "spin", "min 0 max 20", nullptr},
	// JAS end

	{"Ponder", true, "false", "check", "", nullptr},

	{"OwnBook", true, "false", "check", "", nullptr}, {"BookFile", true, "", "string", "", nullptr},
	{"MultiPV", true, "1", "spin", "min 1 max 10", nullptr},

	{"Scorpio Bitbases Pieces", true, "4", "spin", "min 3 max 6", nullptr},
	{"Scorpio Bitbases Path", true, "C:/egbb", "string", "", nullptr},
	{"Scorpio Bitbases Cache Size", true, "64", "spin", "min 4 max 1024", nullptr},
	{"Load Scorpio Bitbases in RAM", true, "4-men", "combo", "var None var 4-men var Smart Load var 5-men", nullptr},

	{"Null Move Pruning", true, "Always", "combo", "var Always var Fail High var Never", nullptr},
	{"Null Move Reduction", true, "3", "spin", "min 1 max 4", nullptr},
	{"Verification Search", true, "Always", "combo", "var Always var Endgame var Never", nullptr},
	{"Verification Reduction", true, "5", "spin", "min 1 max 6", nullptr},

	{"History Pruning", true, "true", "check", "", nullptr},
	{"History Threshold", true, "70", "spin", "min 0 max 100", nullptr},
	{"History Research on Beta", true, "true", "check", "", nullptr},

	{"Rebel Reductions", true, "true", "check", "", nullptr},

	{"Futility Pruning", true, "true", "check", "", nullptr},
	//{ "Quick Futility eval", true, "false", "check", "", nullptr },
	{"Futility Margin", true, "100", "spin", "min 0 max 500", nullptr},
	{"Extended Futility Margin", true, "300", "spin", "min 0 max 900", nullptr},
	{"Futility Pruning Depth", true, "3", "spin", "min 1 max 6", nullptr},

	{"Delta Pruning", true, "true", "check", "", nullptr},
	{"Delta Margin", true, "50", "spin", "min 0 max 500", nullptr},

	{"Quiescence Check Plies", true, "1", "spin", "min 0 max 5", nullptr},

	{"Alt Pawn SQT", true, "false", "check", "", nullptr}, {"Alt Knight SQT", true, "false", "check", "", nullptr},
	{"Alt Bishop SQT", true, "false", "check", "", nullptr},

	{"Chess Knowledge", true, "100", "spin", "min 0 max 500", nullptr},
	{"Piece Activity", true, "100", "spin", "min 0 max 500", nullptr},
	{"Pawn Shelter", true, "100", "spin", "min 0 max 500", nullptr},
	{"Pawn Storm", true, "100", "spin", "min 0 max 500", nullptr},
	{"King Attack", true, "100", "spin", "min 0 max 500", nullptr},
	{"Pawn Structure", true, "100", "spin", "min 0 max 500", nullptr},
	{"Passed Pawns", true, "100", "spin", "min 0 max 500", nullptr},
/*
	{ "knight tropism opening",  	true, "4",    "spin",  "min 0 max 10", nullptr },
	{ "bishop tropism opening",  	true, "2",    "spin",  "min 0 max 10", nullptr },
	{ "rook tropism opening",  		true, "2",    "spin",  "min 0 max 10", nullptr },
	{ "queen tropism opening",  		true, "3",    "spin",  "min 0 max 10", nullptr },

	{ "knight tropism endgame",  	true, "2",    "spin",  "min 0 max 10", nullptr },
	{ "bishop tropism endgame",  	true, "1",    "spin",  "min 0 max 10", nullptr },
	{ "rook tropism endgame",  		true, "1",    "spin",  "min 0 max 10", nullptr },
	{ "queen tropism endgame",  		true, "3",    "spin",  "min 0 max 10", nullptr },
*/
	{"Opening Pawn Value", true, "70", "spin", "min 0 max 10000", nullptr},
	{"Opening Knight Value", true, "325", "spin", "min 0 max 10000", nullptr},
	{"Opening Bishop Value", true, "325", "spin", "min 0 max 10000", nullptr},
	{"Opening Rook Value", true, "500", "spin", "min 0 max 10000", nullptr},
	{"Opening Queen Value", true, "975", "spin", "min 0 max 10000", nullptr},

	{"Endgame Pawn Value", true, "90", "spin", "min 0 max 10000", nullptr},
	{"Endgame Knight Value", true, "315", "spin", "min 0 max 10000", nullptr},
	{"Endgame Bishop Value", true, "315", "spin", "min 0 max 10000", nullptr},
	{"Endgame Rook Value", true, "500", "spin", "min 0 max 10000", nullptr},
	{"Endgame Queen Value", true, "975", "spin", "min 0 max 10000", nullptr},

	{"Bishop Pair Opening", true, "50", "spin", "min 0 max 1000", nullptr},
	{"Bishop Pair Endgame", true, "70", "spin", "min 0 max 1000", nullptr},

	{"Queen Knight combo", true, "15", "spin", "min 0 max 1000", nullptr},
	{"Rook Bishop combo", true, "15", "spin", "min 0 max 1000", nullptr},

	{"Bad Trade Value", true, "50", "spin", "min 0 max 1000", nullptr},

	{"Contempt Factor", true, "0", "spin", "min -1000 max 1000", nullptr},

#ifdef TMO_OPT
//test
{"PawnAmountBonusOpening0", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusOpening1", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusOpening2", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusOpening3", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusOpening4", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusOpening5", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusOpening6", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusOpening7", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusOpening8", true, "0", "spin", "min -100 max 100", nullptr},

{"PawnAmountBonusEndgame0", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusEndgame1", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusEndgame2", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusEndgame3", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusEndgame4", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusEndgame5", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusEndgame6", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusEndgame7", true, "0", "spin", "min -100 max 100", nullptr},
{"PawnAmountBonusEndgame8", true, "0", "spin", "min -100 max 100", nullptr},

{"DoubledOpening1", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledOpening2", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledOpening3", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledOpening4", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledOpening5", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledOpening6", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledOpening7", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledOpening8", true, "0", "spin", "min -100 max 100", nullptr},

{"DoubledEndgame1", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledEndgame2", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledEndgame3", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledEndgame4", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledEndgame5", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledEndgame6", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledEndgame7", true, "0", "spin", "min -100 max 100", nullptr},
{"DoubledEndgame8", true, "0", "spin", "min -100 max 100", nullptr},

{"IsolatedOpening1", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpening2", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpening3", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpening4", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpening5", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpening6", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpening7", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpening8", true, "0", "spin", "min -100 max 100", nullptr},

{"IsolatedOpeningOpen1", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpeningOpen2", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpeningOpen3", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpeningOpen4", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpeningOpen5", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpeningOpen6", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpeningOpen7", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedOpeningOpen8", true, "0", "spin", "min -100 max 100", nullptr},

{"IsolatedEndgame1", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedEndgame2", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedEndgame3", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedEndgame4", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedEndgame5", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedEndgame6", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedEndgame7", true, "0", "spin", "min -100 max 100", nullptr},
{"IsolatedEndgame8", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpening1", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpening2", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpening3", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpening4", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpening5", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpening6", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpening7", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpening8", true, "0", "spin", "min -100 max 100", nullptr},

{"BackwardOpeningOpen1", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpeningOpen2", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpeningOpen3", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpeningOpen4", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpeningOpen5", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpeningOpen6", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpeningOpen7", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardOpeningOpen8", true, "0", "spin", "min -100 max 100", nullptr},

{"BackwardEndgame1", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardEndgame2", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardEndgame3", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardEndgame4", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardEndgame5", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardEndgame6", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardEndgame7", true, "0", "spin", "min -100 max 100", nullptr},
{"BackwardEndgame8", true, "0", "spin", "min -100 max 100", nullptr},
#endif

	{nullptr, false, nullptr, nullptr, nullptr, nullptr,},};

// prototypes

static option_t *option_find(const char var[]);

// functions

// option_init()

void option_init() {
	for (option_t *opt = &Option[0]; opt->var != nullptr; ++opt)
		option_set(opt->var, opt->init);
}

// option_list()

void option_list() {
	for (option_t *opt = &Option[0]; opt->var != nullptr; ++opt)
		if (opt->declare) {
			if (opt->extra != nullptr && *opt->extra != '\0')
				send("option name %s type %s default %s %s", opt->var, opt->type, opt->val, opt->extra);
			else
				send("option name %s type %s default %s", opt->var, opt->type, opt->val);
		}
}

// option_set()

bool option_set(const char var[], const char val[]) {

	ASSERT(var != nullptr);
	ASSERT(val != nullptr);

	option_t *opt = option_find(var);
	if (opt == nullptr) return false;

	my_string_set(&opt->val, val);
	return true;
}

// option_get()

const char *option_get(const char var[]) {

	ASSERT(var != nullptr);

	option_t *opt = option_find(var);
	if (opt == nullptr) my_fatal("option_get(): unknown option \"%s\"\n", var);

	return opt->val;
}

// option_get_bool()

bool option_get_bool(const char var[]) {

	const char *val = option_get(var);

	if (false) {
	} else if (my_string_equal(val, "true") || my_string_equal(val, "yes") || my_string_equal(val, "1")) {
		return true;
	} else if (my_string_equal(val, "false") || my_string_equal(val, "no") || my_string_equal(val, "0")) {
		return false;
	}

	ASSERT(false);

	return false;
}

// option_get_int()

S64 option_get_int(const char var[]) {
	const char *val = option_get(var);
	return atoll(val);
}

S64 option_get_s64(const char var[]) {
	const char *val = option_get(var);
	return strtoll(val, nullptr, 10);
}

S32 option_get_s32(const char var[]) {
	const char *val = option_get(var);
	return strtol(val, nullptr, 10);
}

S16 option_get_s16(const char var[]) {
	const char *val = option_get(var);
	return static_cast<S16>(strtol(val, nullptr, 10));
}

S8 option_get_s8(const char var[]) {
	const char *val = option_get(var);
	return static_cast<S8>(strtol(val, nullptr, 10));
}


U64 option_get_u64(const char var[]) {
	const char *val = option_get(var);
	return strtoull(val, nullptr, 10);
}

U32 option_get_u32(const char var[]) {
	const char *val = option_get(var);
	return strtoul(val, nullptr, 10);
}

U16 option_get_u16(const char var[]) {
	const char *val = option_get(var);
	return static_cast<U16>(strtoul(val, nullptr, 10));
}

U8 option_get_u8(const char var[]) {
	const char *val = option_get(var);
	return static_cast<U8>(strtoul(val, nullptr, 10));
}

// option_get_string()

const char *option_get_string(const char var[]) {
	const char *val = option_get(var);
	return val;
}

// option_find()

static option_t *option_find(const char var[]) {

	ASSERT(var != nullptr);

	for (option_t *opt = &Option[0]; opt->var != nullptr; ++opt)
		if (my_string_equal(opt->var, var)) return opt;

	return nullptr;
}

// end of option.cpp
