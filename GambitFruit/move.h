// move.h

#ifndef MOVE_H
#define MOVE_H

// includes

#include <cstdint>
#include "board.h"
#include "util.h"

// constants

constexpr U16 MoveNone = 0;  // HACK: a1a1 cannot be a legal move
constexpr U16 MoveNull = 11; // HACK: a1d2 cannot be a legal move

constexpr U16 MoveNormal = 0 << 14;
constexpr U16 MoveCastle = 1 << 14;
constexpr U16 MovePromote = 2 << 14;
constexpr U16 MoveEnPassant = 3 << 14;
constexpr U16 MoveFlags = 3 << 14;

constexpr U16 MovePromoteKnight = MovePromote | (0 << 12);
constexpr U16 MovePromoteBishop = MovePromote | (1 << 12);
constexpr U16 MovePromoteRook = MovePromote | (2 << 12);
constexpr U16 MovePromoteQueen = MovePromote | (3 << 12);

constexpr U16 MoveAllFlags = 0xF << 12; //??

const char NullMoveString[] = "null"; // "0000" in UCI

// macros

#define MOVE_MAKE(from, to)              ((SQUARE_TO_64(from)<<6)|SQUARE_TO_64(to))
#define MOVE_MAKE_FLAGS(from, to, flags) ((SQUARE_TO_64(from)<<6)|SQUARE_TO_64(to)|(flags))

#define MOVE_FROM(move)                (SQUARE_FROM_64(((move)>>6)&077))
#define MOVE_TO(move)                  (SQUARE_FROM_64((move)&077))

#define MOVE_IS_SPECIAL(move)          (((move)&MoveFlags)!=MoveNormal)
#define MOVE_IS_PROMOTE(move)          (((move)&MoveFlags)==MovePromote)
#define MOVE_IS_EN_PASSANT(move)       (((move)&MoveFlags)==MoveEnPassant)
#define MOVE_IS_CASTLE(move)           (((move)&MoveFlags)==MoveCastle)
#define MOVE_PROMOTE_PIECE(move)       (((move)&MovePromoteQueen)>>12)

#define MOVE_PIECE(move, board)         ((board)->square[MOVE_FROM(move)])

// types

typedef U16 mv_t;

// functions

extern bool move_is_ok(U16 move);

extern U16 move_promote(U16 move);

extern U16 move_order(U16 move);

extern bool move_is_capture(U16 move, const board_t *board);
extern bool move_is_under_promote(U16 move);
extern bool move_is_tactical(U16 move, const board_t *board);

extern U16 move_capture(U16 move, const board_t *board);

extern bool move_to_string(U16 move, char string[], S32 size);
extern U16 move_from_string(const char string[], const board_t *board);

#endif // !defined MOVE_H

// end of move.h
