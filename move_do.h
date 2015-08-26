// move_do.h

#ifndef MOVE_DO_H
#define MOVE_DO_H

// includes

#include <cstdint>
#include "board.h"
#include "util.h"

// types

struct undo_t {

	uint_fast64_t key;
	uint_fast64_t pawn_key;
	uint_fast64_t material_key;

	int_fast32_t capture_square;
	int_fast32_t capture_piece;
	int_fast32_t capture_pos;

	int_fast32_t pawn_pos;


	int_fast32_t flags;
	int_fast32_t ep_square;

	int_fast32_t cap_sq;

	int_fast32_t opening;
	int_fast32_t endgame;

	int_fast16_t ply_nb;

	bool capture;
	bool turn;
};

// functions

extern void move_do_init();

extern void move_do(board_t *board, uint_fast16_t move, undo_t *undo);
extern void move_undo(board_t *board, uint_fast16_t move, const undo_t *undo);

extern void move_do_null(board_t *board, undo_t *undo);
extern void move_undo_null(board_t *board, const undo_t *undo);

#endif // !defined MOVE_DO_H

// end of move_do.h
