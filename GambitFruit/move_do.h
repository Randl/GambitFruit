// move_do.h

#ifndef MOVE_DO_H
#define MOVE_DO_H

// includes

#include <cstdint>
#include "board.h"
#include "util.h"

// types

struct undo_t {

    U64 key;
    U64 pawn_key;
    U64 material_key;

    S32 capture_square;
    S32 capture_piece;
    S32 capture_pos;

    S32 pawn_pos;


    S32 flags;
    S32 ep_square;

    S32 cap_sq;

    S32 opening;
    S32 endgame;

    U16 ply_nb;

    bool capture;
    bool turn;
};

// functions

extern void move_do_init();

extern void move_do(board_t *board, U16 move, undo_t *undo);
extern void move_undo(board_t *board, U16 move, const undo_t *undo);

extern void move_do_null(board_t *board, undo_t *undo);
extern void move_undo_null(board_t *board, const undo_t *undo);

#endif // !defined MOVE_DO_H

// end of move_do.h
