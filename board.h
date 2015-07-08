
// board.h

#ifndef BOARD_H
#define BOARD_H

// includes

#include <cstdint>
#include "colour.h"
#include "piece.h"
#include "square.h"
#include "util.h"

// constants

constexpr int_fast32_t Empty = 0;
constexpr int_fast32_t Edge = Knight64; // HACK: uncoloured knight

constexpr int_fast32_t WP = WhitePawn256;
constexpr int_fast32_t WN = WhiteKnight256;
constexpr int_fast32_t WB = WhiteBishop256;
constexpr int_fast32_t WR = WhiteRook256;
constexpr int_fast32_t WQ = WhiteQueen256;
constexpr int_fast32_t WK = WhiteKing256;

constexpr int_fast32_t BP = BlackPawn256;
constexpr int_fast32_t BN = BlackKnight256;
constexpr int_fast32_t BB = BlackBishop256;
constexpr int_fast32_t BR = BlackRook256;
constexpr int_fast32_t BQ = BlackQueen256;
constexpr int_fast32_t BK = BlackKing256;

constexpr int_fast32_t FlagsNone = 0;
constexpr int_fast32_t FlagsWhiteKingCastle  = 1 << 0;
constexpr int_fast32_t FlagsWhiteQueenCastle = 1 << 1;
constexpr int_fast32_t FlagsBlackKingCastle  = 1 << 2;
constexpr int_fast32_t FlagsBlackQueenCastle = 1 << 3;

constexpr int_fast32_t StackSize = 4096;

// macros

#define KING_POS(board,colour) ((board)->piece[colour][0])

// types

struct board_t {

   int_fast32_t piece_material[ColourNb]; // Thomas
     	
   int_fast32_t square[SquareNb];
   int_fast32_t pos[SquareNb];

   sq_t piece[ColourNb][17]; // was 32
   int_fast32_t piece_size[ColourNb];

   sq_t pawn[ColourNb][9]; // was 16
   int_fast32_t pawn_size[ColourNb];

   int_fast32_t piece_nb;
   int_fast32_t number[12]; // was 16

   int_fast32_t pawn_file[ColourNb][FileNb];

   bool turn;
   int_fast32_t flags;
   int_fast32_t ep_square;
   int_fast32_t ply_nb;
   int_fast32_t sp; // TODO: MOVE ME?

   int_fast32_t cap_sq;

   int_fast32_t opening;
   int_fast32_t endgame;
   int_fast32_t pvalue; //Ryan

   uint_fast64_t key;
   uint_fast64_t pawn_key;
   uint_fast64_t material_key;

   uint_fast64_t stack[StackSize];
};

// functions

extern bool board_is_ok         (const board_t * board);

extern void board_clear         (board_t * board);
extern void board_copy          (board_t * dst, const board_t * src);

extern void board_init_list     (board_t * board);

extern bool board_is_legal      (const board_t * board);
extern bool board_is_check      (const board_t * board);
extern bool board_is_mate       (const board_t * board);
extern bool board_is_stalemate  (board_t * board);

extern bool board_is_repetition (const board_t * board);

extern int_fast32_t  board_material      (const board_t * board);
extern int_fast32_t  board_opening       (const board_t * board);
extern int_fast32_t  board_endgame       (const board_t * board);

#endif // !defined BOARD_H

// end of board.h