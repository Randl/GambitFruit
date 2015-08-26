// sort.h

#ifndef SORT_H
#define SORT_H

// includes

#include "attack.h"
#include "board.h"
#include "list.h"
#include "util.h"

// types

struct fail_high_stats_t {
	uint_fast64_t tried;
	uint_fast64_t success;
};


struct sort_t {
	board_t        *board;
	const attack_t *attack;
	list_t         list[1];
	list_t         bad[1];
	int_fast32_t   depth;
	int_fast32_t   height;
	int_fast32_t   trans_killer;
	int_fast32_t   killer_1;
	int_fast32_t   killer_2;
	int_fast32_t   killer_3;
	int_fast32_t   killer_4;
	int_fast32_t   gen;
	int_fast32_t   test;
	int_fast32_t   pos;
	int_fast32_t   value;
};

// functions

extern void sort_init();

extern void         sort_init(sort_t *sort, board_t *board, const attack_t *attack, int_fast32_t depth,
							  int_fast32_t height,
							  int_fast32_t trans_killer);
extern int_fast32_t sort_next(sort_t *sort);

extern void         sort_init_qs(sort_t *sort, board_t *board, const attack_t *attack, bool check);
extern int_fast32_t sort_next_qs(sort_t *sort);

extern void good_move(uint_fast16_t move, const board_t *board, int_fast32_t depth, int_fast32_t height);

extern void history_good(uint_fast16_t move, const board_t *board);
extern void history_bad(uint_fast16_t move, const board_t *board);
extern void history_very_bad(uint_fast16_t move, const board_t *board);

extern bool history_reduction(uint_fast16_t move, const board_t *board);
extern void history_tried(uint_fast16_t move, const board_t *board);
extern void history_success(uint_fast16_t move, const board_t *board);

extern void note_moves(list_t *list, const board_t *board, int_fast32_t height, int_fast32_t trans_killer);

#endif // !defined SORT_H

// end of sort.h
