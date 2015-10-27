// pawn.h

#ifndef PAWN_H
#define PAWN_H

// includes

#include <array>
#include "board.h"
#include "colour.h"
#include "util.h"

// macros

#define BIT(n)       (BitEQ[n])

#define BIT_FIRST(b) (BitFirst[b])
#define BIT_LAST(b)  (BitLast[b])
#define BIT_COUNT(b) (BitCount[b])

// constants

constexpr S16 BackRankFlag = 1 << 0;

constexpr S16 BadPawnFileA = 1 << 0;
constexpr S16 BadPawnFileB = 1 << 1;
constexpr S16 BadPawnFileC = 1 << 2;
constexpr S16 BadPawnFileD = 1 << 3;
constexpr S16 BadPawnFileE = 1 << 4;
constexpr S16 BadPawnFileF = 1 << 5;
constexpr S16 BadPawnFileG = 1 << 6;
constexpr S16 BadPawnFileH = 1 << 7;

// types

struct pawn_info_t {
    U32 lock;
    S16 opening;
    S16 endgame;
    std::array<U8, ColourNb> flags;
    std::array<U8, ColourNb> passed_bits;
    std::array<U8, ColourNb> single_file;
    std::array<U8, ColourNb> badpawns; // Ryan
    std::array<U8, ColourNb> wsp; // Ryan
};

// variables

extern std::array<S32, 16> BitEQ;
extern std::array<S32, 16> BitLT;
extern std::array<S32, 16> BitLE;
extern std::array<S32, 16> BitGT;
extern std::array<S32, 16> BitGE;

extern std::array<S32, 0x100> BitFirst;
extern std::array<S32, 0x100> BitLast;
extern std::array<S32, 0x100> BitCount;
extern std::array<S32, 0x100> BitRev;

// functions

extern void pawn_init_bit();
extern void pawn_init();

extern void pawn_alloc();
extern void pawn_clear();

extern void pawn_get_info(pawn_info_t *info, const board_t *board);

extern S32 quad(S32 y_min, S32 y_max, S32 x);

#endif // !defined PAWN_H

// end of pawn.h
