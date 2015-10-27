// board.h

#ifndef BOARD_H
#define BOARD_H

// includes

#include <cstdint>
#include <vector>
#include "colour.h"
#include "piece.h"
#include "square.h"
#include "util.h"

// constants

constexpr S32 Empty = 0;
constexpr S32 Edge = Knight64; // HACK: uncoloured knight

constexpr S32 WP = WhitePawn256;
constexpr S32 WN = WhiteKnight256;
constexpr S32 WB = WhiteBishop256;
constexpr S32 WR = WhiteRook256;
constexpr S32 WQ = WhiteQueen256;
constexpr S32 WK = WhiteKing256;

constexpr S32 BP = BlackPawn256;
constexpr S32 BN = BlackKnight256;
constexpr S32 BB = BlackBishop256;
constexpr S32 BR = BlackRook256;
constexpr S32 BQ = BlackQueen256;
constexpr S32 BK = BlackKing256;

constexpr S8 FlagsNone = 0;
constexpr S8 FlagsWhiteKingCastle = 1 << 0;
constexpr S8 FlagsWhiteQueenCastle = 1 << 1;
constexpr S8 FlagsBlackKingCastle = 1 << 2;
constexpr S8 FlagsBlackQueenCastle = 1 << 3;

constexpr U16 StackSize = 4096;

// macros

#define KING_POS(board, colour) ((board)->piece[colour][0])

// types

struct board_t {

    U64 key;
    U64 pawn_key;
    U64 material_key;

    std::vector<U64> stack;

    std::array<S16, ColourNb> piece_material; // Thomas

    std::array<S32, SquareNb> square;
    std::array<S8, SquareNb> pos;

    std::array<std::vector<sq_t>, ColourNb> piece;
    std::array<std::vector<sq_t>, ColourNb> pawn;

    std::array<S32, 12> number; // was 16

    std::array<std::array<S8, FileNb>, ColourNb> pawn_file;


    S32 flags;
    S32 ep_square;

    S32 cap_sq;

    S32 opening;
    S32 endgame;
    S32 pvalue; //Ryan

    U16 ply_nb;
    S8 piece_nb;  //remove me?

    bool turn;
};

// functions

extern bool board_is_ok(const board_t *board);

extern void board_clear(board_t *board);
extern void board_copy(board_t *dst, const board_t *src);

extern void board_init_list(board_t *board);

extern bool board_is_legal(const board_t *board);
extern bool board_is_check(const board_t *board);
extern bool board_is_mate(const board_t *board);
extern bool board_is_stalemate(board_t *board);

extern bool board_is_repetition(const board_t *board);

extern S32 board_material(const board_t *board);
extern S32 board_opening(const board_t *board);
extern S32 board_endgame(const board_t *board);

#endif // !defined BOARD_H

// end of board.h
