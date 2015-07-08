
// hash.h

#ifndef HASH_H
#define HASH_H

// includes

#include <cstdint>
#include "board.h"
#include "util.h"

// macros

#define KEY_INDEX(key) (uint_fast32_t(key))
#define KEY_LOCK(key)  (uint_fast32_t((key)>>32))

// constants

const int_fast32_t RandomPiece     =   0; // 12 * 64
const int_fast32_t RandomCastle    = 768; // 4
const int_fast32_t RandomEnPassant = 772; // 8
const int_fast32_t RandomTurn      = 780; // 1

// variables

extern uint_fast64_t Castle64[16];

// functions

extern void   hash_init         ();

extern uint_fast64_t hash_key          (const board_t * board);
extern uint_fast64_t hash_pawn_key     (const board_t * board);
extern uint_fast64_t hash_material_key (const board_t * board);

extern uint_fast64_t hash_piece_key    (int_fast32_t piece, int_fast32_t square);
extern uint_fast64_t hash_castle_key   (int_fast32_t flags);
extern uint_fast64_t hash_ep_key       (int_fast32_t square);
extern uint_fast64_t hash_turn_key     (int_fast32_t colour);

#endif // !defined HASH_H

// end of hash.h

