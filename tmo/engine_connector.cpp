//
// Created by Evgenii on 29.08.2015.
//

#include "engine_connector.h"
#include "../GambitFruit/attack.h"
#include "../GambitFruit/book.h"
#include "../GambitFruit/hash.h"
#include "../GambitFruit/move_do.h"
#include "../GambitFruit/option.h"
#include "../GambitFruit/pawn.h"
#include "../GambitFruit/protocol.h"
#include "../GambitFruit/random.h"
#include "../GambitFruit/trans.h"
#include "../GambitFruit/value.h"
#include "../GambitFruit/search.h"
#include "../GambitFruit/fen.h"
#include "../GambitFruit/search_full.h"
#include "../GambitFruit/sort.h"
#include "../GambitFruit/eval.h"
#include "../GambitFruit/material.h"

int_fast32_t call_q_search(std::string fen) {
	// init
	search_clear();
	eval_init();
	material_init();
	material_alloc();
	mv_t pv[HeightMax];

	S32 a = -ValueInf, b = +ValueInf;

	board_from_fen(SearchInput->board, fen.c_str());
	return full_quiescence(SearchInput->board, a, b, 0, 0, pv);
}

void engine_init() {
	util_init();
	my_random_init(); // for opening book

	option_init();

	square_init();
	piece_init();
	pawn_init_bit();
	value_init();
	vector_init();
	attack_init();
	move_do_init();

	random_init();
	hash_init();

	trans_init(Trans);
	book_init();

	init();
	sort_init();

	search_clear();

	engine_set_options({"Hash"}, {0});
}
void engine_set_options(std::vector<std::string> options, std::vector<S16> params) {
	for (int i = 0; i < params.size(); ++i) {
		std::string num = std::to_string(int_fast32_t(params[i]));
		std::string tmp = "setoption name " + options[i] + " value " + num;
		parse_setoption(const_cast<char *>(tmp.c_str()));
	}
}
