// hash.h

#ifndef HASH_H
#define HASH_H

// includes

#include <cstdint>
#include <array>
#include "board.h"
#include "util.h"

// macros

#define KEY_INDEX(key) (U32(key))
#define KEY_LOCK(key)  (U32((key)>>32))

// constants

constexpr S32 RandomPiece = 0; // 12 * 64
constexpr S32 RandomCastle = 768; // 4
constexpr S32 RandomEnPassant = 772; // 8
constexpr S32 RandomTurn = 780; // 1

// variables

extern std::array<U64, 16> Castle64;

// functions

extern void hash_init();

extern U64 hash_key(const board_t *board);
extern U64 hash_pawn_key(const board_t *board);
extern U64 hash_material_key(const board_t *board);

extern U64 hash_piece_key(S32 piece, S32 square);
extern U64 hash_castle_key(S32 flags);
extern U64 hash_ep_key(S32 square);
extern U64 hash_turn_key(S8 colour);

#endif // !defined HASH_H

// end of hash.h
