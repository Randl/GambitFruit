// move.h

#ifndef MOVE_H
#define MOVE_H

// includes

#include <cstdint>
#include "board.h"
#include "util.h"

// constants

constexpr int_fast32_t MoveNone = 0;  // HACK: a1a1 cannot be a legal move
constexpr int_fast32_t MoveNull = 11; // HACK: a1d2 cannot be a legal move

constexpr int_fast32_t MoveNormal    = 0 << 14;
constexpr int_fast32_t MoveCastle    = 1 << 14;
constexpr int_fast32_t MovePromote   = 2 << 14;
constexpr int_fast32_t MoveEnPassant = 3 << 14;
constexpr int_fast32_t MoveFlags     = 3 << 14;

constexpr int_fast32_t MovePromoteKnight = MovePromote | (0 << 12);
constexpr int_fast32_t MovePromoteBishop = MovePromote | (1 << 12);
constexpr int_fast32_t MovePromoteRook   = MovePromote | (2 << 12);
constexpr int_fast32_t MovePromoteQueen  = MovePromote | (3 << 12);

constexpr int_fast32_t MoveAllFlags = 0xF << 12;

const char NullMoveString[] = "null"; // "0000" in UCI

// macros

#define MOVE_MAKE(from, to)             ((SQUARE_TO_64(from)<<6)|SQUARE_TO_64(to))
#define MOVE_MAKE_FLAGS(from, to, flags) ((SQUARE_TO_64(from)<<6)|SQUARE_TO_64(to)|(flags))

#define MOVE_FROM(move)                (SQUARE_FROM_64(((move)>>6)&077))
#define MOVE_TO(move)                  (SQUARE_FROM_64((move)&077))

#define MOVE_IS_SPECIAL(move)          (((move)&MoveFlags)!=MoveNormal)
#define MOVE_IS_PROMOTE(move)          (((move)&MoveFlags)==MovePromote)
#define MOVE_IS_EN_PASSANT(move)       (((move)&MoveFlags)==MoveEnPassant)
#define MOVE_IS_CASTLE(move)           (((move)&MoveFlags)==MoveCastle)

#define MOVE_PIECE(move, board)         ((board)->square[MOVE_FROM(move)])

// types

typedef uint_fast16_t mv_t;

// functions

extern bool move_is_ok(int_fast32_t move);

extern int_fast32_t move_promote(int_fast32_t move);

extern int_fast32_t move_order(int_fast32_t move);

extern bool move_is_capture(int_fast32_t move, const board_t *board);
extern bool move_is_under_promote(int_fast32_t move);
extern bool move_is_tactical(int_fast32_t move, const board_t *board);

extern int_fast32_t move_capture(int_fast32_t move, const board_t *board);

extern bool         move_to_string(int_fast32_t move, char string[], int_fast32_t size);
extern int_fast32_t move_from_string(const char string[], const board_t *board);

#endif // !defined MOVE_H

// end of move.h
