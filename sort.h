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
    U64 tried;
    U64 success;
};


struct sort_t {
	board_t        *board;
	const attack_t *attack;
	list_t         list[1];
	list_t         bad[1];
    S32 depth;
    S32 height;
    S32 trans_killer;
    S32 killer_1;
    S32 killer_2;
    S32 killer_3;
    S32 killer_4;
    S32 gen;
    S32 test;
    S32 pos;
    S32 value;
};

// functions

extern void sort_init();

extern void sort_init(sort_t *sort, board_t *board, const attack_t *attack, S32 depth, S32 height, S32 trans_killer);
extern S32 sort_next(sort_t *sort);

extern void         sort_init_qs(sort_t *sort, board_t *board, const attack_t *attack, bool check);
extern S32 sort_next_qs(sort_t *sort);

extern void good_move(U16 move, const board_t *board, S32 depth, S32 height);

extern void history_good(U16 move, const board_t *board);
extern void history_bad(U16 move, const board_t *board);
extern void history_very_bad(U16 move, const board_t *board);

extern bool history_reduction(U16 move, const board_t *board);
extern void history_tried(U16 move, const board_t *board);
extern void history_success(U16 move, const board_t *board);

extern void note_moves(list_t *list, const board_t *board, S32 height, S32 trans_killer);

#endif // !defined SORT_H

// end of sort.h
