// move_gen.h

#ifndef MOVE_GEN_H
#define MOVE_GEN_H

// includes

#include <cstdint>
#include "attack.h"
#include "board.h"
#include "list.h"
#include "util.h"

// variables
const std::array<int_fast8_t, 8> king_moves   = {-17, -16, -15, -1, 1, 15, 16, 17};
const std::array<int_fast8_t, 8> knight_moves = {-33, -31, -18, -14, 14, 18, 31, 33};
const std::array<int_fast8_t, 4> bishop_moves = {-17, -15, 15, 17};
const std::array<int_fast8_t, 4> rook_moves   = {-16, -1, 1, 16};
const std::array<int_fast8_t, 8> queen_moves  = {-17, -16, -15, -1, 1, 15, 16, 17};

// functions

extern void gen_legal_moves(list_t *list, board_t *board);

extern void gen_moves(list_t *list, const board_t *board);
extern void gen_captures(list_t *list, const board_t *board);
extern void gen_quiet_moves(list_t *list, const board_t *board);

extern void add_pawn_move(list_t *list, int_fast32_t from, int_fast32_t to);
extern void                      add_promote(list_t *list, uint_fast16_t move);

#endif // !defined MOVE_GEN_H

// end of move_gen.h

