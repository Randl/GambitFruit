
// option.cpp

// includes

#include <cstdlib>

#include "option.h"
#include "protocol.h"
#include "util.h"

// types

struct option_t {
	const char * var;
	bool declare;
	const char * init;
	const char * type;
	const char * extra;
	const char * val;
};

// variables
static option_t Option[] = {

	{ "Hash", true, "16", "spin", "min 4 max 1024", nullptr },

	// JAS
	// search X seconds for the best move, equal to "go movetime"
	{ "Search Time",  true, "0",   "spin",  "min 0 max 3600", nullptr },
	// search X plies deep, equal to "go depth"
	{ "Search Depth",  true, "0",   "spin",  "min 0 max 20", nullptr },
	// JAS end

	{ "Ponder", true, "false", "check", "", nullptr },

	{ "OwnBook",  true, "false",           "check",  "", nullptr },
	{ "BookFile", true, "", "string", "", nullptr },
	{ "MultiPV", true, "1", "spin",  "min 1 max 10", nullptr },

	{ "Bitbase Pieces", true, "4", "spin",  "min 3 max 6", nullptr },
	{ "Bitbase Path", true, "C:/egbb", "string", "", nullptr },
	{ "Bitbase Cache Size", true, "16", "spin",  "min 4 max 1024", nullptr },

	{ "NullMove Pruning",       true, "Always", "combo", "var Always var Fail High var Never", nullptr },
	{ "NullMove Reduction",     true, "3",         "spin",  "min 1 max 4", nullptr },
	{ "Verification Search",    true, "Always",   "combo", "var Always var Endgame var Never", nullptr },
	{ "Verification Reduction", true, "5",         "spin",  "min 1 max 6", nullptr },

	{ "History Pruning",     true, "true", "check", "", nullptr },
	{ "History Threshold",   true, "70",   "spin",  "min 0 max 100", nullptr },
	{ "History Research on Beta",     true, "true", "check", "", nullptr },

	{ "Rebel Reductions",     true, "true", "check", "", nullptr },

	{ "Futility Pruning", true, "true", "check", "", nullptr },
	//{ "Quick Futility eval", true, "false", "check", "", nullptr },
	{ "Futility Margin",  true, "100",   "spin",  "min 0 max 500", nullptr },
	{ "Extended Futility Margin",  true, "300",   "spin",  "min 0 max 900", nullptr },
	{ "Futility Pruning Depth",     true, "3",         "spin",  "min 1 max 6", nullptr },
   
	{ "Delta Pruning", true, "true", "check", "", nullptr },
	{ "Delta Margin",  true, "50",    "spin",  "min 0 max 500", nullptr },

	{ "Quiescence Check Plies", true, "1", "spin", "min 0 max 5", nullptr },

	{ "Alt Pawn SQT", true, "false", "check", "", nullptr },
	{ "Alt Knight SQT", true, "false", "check", "", nullptr },
	{ "Alt Bishop SQT", true, "false", "check", "", nullptr },

	{ "Chess Knowledge", true, "100", "spin", "min 0 max 500", nullptr },
	{ "Piece Activity",  true, "100", "spin", "min 0 max 500", nullptr },
	{ "Pawn Shelter",    true, "100", "spin", "min 0 max 500", nullptr },
	{ "Pawn Storm",      true, "100", "spin", "min 0 max 500", nullptr },
	{ "King Attack",     true, "100", "spin", "min 0 max 500", nullptr },
	{ "Pawn Structure",  true, "100", "spin", "min 0 max 500", nullptr },
	{ "Passed Pawns",    true, "100", "spin", "min 0 max 500", nullptr },
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
	{ "Opening Pawn Value",	true, "70", "spin", "min 0 max 10000", nullptr },
	{ "Opening Knight Value",	true, "325", "spin", "min 0 max 10000", nullptr },
	{ "Opening Bishop Value",	true, "325", "spin", "min 0 max 10000", nullptr },
	{ "Opening Rook Value",	true, "500", "spin", "min 0 max 10000", nullptr },
	{ "Opening Queen Value",	true, "975", "spin", "min 0 max 10000", nullptr },

	{ "Endgame Pawn Value",	true, "90", "spin", "min 0 max 10000", nullptr },
	{ "Endgame Knight Value",	true, "315", "spin", "min 0 max 10000", nullptr },
	{ "Endgame Bishop Value",	true, "315", "spin", "min 0 max 10000", nullptr },
	{ "Endgame Rook Value",	true, "500", "spin", "min 0 max 10000", nullptr },
	{ "Endgame Queen Value",	true, "975", "spin", "min 0 max 10000", nullptr },

	{ "Bishop Pair Opening",	true, "50", "spin", "min 0 max 1000", nullptr },
	{ "Bishop Pair Endgame",	true, "70", "spin", "min 0 max 1000", nullptr },

	{ "Queen Knight combo", 	true, "15", "spin", "min 0 max 1000", nullptr },
	{ "Rook Bishop combo",	true, "15", "spin", "min 0 max 1000", nullptr },

	{ "Bad Trade Value",  true, "50",    "spin",  "min 0 max 1000", nullptr }, 

	{ "Contempt Factor",	true, "0", "spin", "min -1000 max 1000", nullptr },


	{ nullptr, false, nullptr, nullptr, nullptr, nullptr, },
};

// prototypes

static option_t * option_find (const char var[]);

// functions

// option_init()

void option_init() {
	for (option_t *opt = &Option[0]; opt->var != nullptr; ++opt)
		option_set(opt->var,opt->init);
}

// option_list()

void option_list() {
	for (option_t *opt = &Option[0]; opt->var != nullptr; ++opt) 
		if (opt->declare) 
			if (opt->extra != nullptr && *opt->extra != '\0') 
				send("option name %s type %s default %s %s",opt->var,opt->type,opt->val,opt->extra);
			else 
				send("option name %s type %s default %s",opt->var,opt->type,opt->val);
}

// option_set()

bool option_set(const char var[], const char val[]) {

	ASSERT(var!=nullptr);
	ASSERT(val!=nullptr);

	option_t *opt = option_find(var);
	if (opt == nullptr) return false;

	my_string_set(&opt->val,val);
	return true;
}

// option_get()

const char * option_get(const char var[]) {
	
	ASSERT(var!=nullptr);

	option_t *opt = option_find(var);
	if (opt == nullptr) my_fatal("option_get(): unknown option \"%s\"\n",var);

	return opt->val;
}

// option_get_bool()

bool option_get_bool(const char var[]) {

	const char *val = option_get(var);

	if (false) {
	} else if (my_string_equal(val,"true") || my_string_equal(val,"yes") || my_string_equal(val,"1")) {
		return true;
	} else if (my_string_equal(val,"false") || my_string_equal(val,"no") || my_string_equal(val,"0")) {
		return false;
	}

	ASSERT(false);

	return false;
}

// option_get_int()

int_fast32_t option_get_int(const char var[]) {
	const char * val = option_get(var);
	return atoi(val);
}

// option_get_string()

const char * option_get_string(const char var[]) {
	const char * val = option_get(var);
	return val;
}

// option_find()

static option_t * option_find(const char var[]) {

	ASSERT(var!=nullptr);

	for (option_t *opt = &Option[0]; opt->var != nullptr; ++opt)
		if (my_string_equal(opt->var,var)) return opt;

	return nullptr;
}

// end of option.cpp