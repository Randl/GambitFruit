
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

constexpr int_fast32_t BackRankFlag = 1 << 0;

constexpr int_fast32_t BadPawnFileA = 1 << 0;
constexpr int_fast32_t BadPawnFileB = 1 << 1;
constexpr int_fast32_t BadPawnFileC = 1 << 2;
constexpr int_fast32_t BadPawnFileD = 1 << 3;
constexpr int_fast32_t BadPawnFileE = 1 << 4;
constexpr int_fast32_t BadPawnFileF = 1 << 5;
constexpr int_fast32_t BadPawnFileG = 1 << 6;
constexpr int_fast32_t BadPawnFileH = 1 << 7;

// types

struct pawn_info_t {
   uint_fast32_t lock;
   int_fast16_t opening;
   int_fast16_t endgame;
   uint_fast8_t flags[ColourNb];
   uint_fast8_t passed_bits[ColourNb];
   uint_fast8_t single_file[ColourNb];
   uint_fast8_t badpawns[ColourNb]; // Ryan
   uint_fast8_t wsp[ColourNb]; // Ryan
};

// variables

extern std::array<int_fast32_t, 16> BitEQ;
extern std::array<int_fast32_t, 16> BitLT;
extern std::array<int_fast32_t, 16> BitLE;
extern std::array<int_fast32_t, 16> BitGT;
extern std::array<int_fast32_t, 16> BitGE;

extern std::array<int_fast32_t, 0x100> BitFirst;
extern std::array<int_fast32_t, 0x100> BitLast;
extern std::array<int_fast32_t, 0x100> BitCount;
extern std::array<int_fast32_t, 0x100> BitRev;

// functions

extern void pawn_init_bit ();
extern void pawn_init     ();

extern void pawn_alloc    ();
extern void pawn_clear    ();

extern void pawn_get_info (pawn_info_t * info, const board_t * board);

extern int_fast32_t  quad          (int_fast32_t y_min, int_fast32_t y_max, int_fast32_t x);

#endif // !defined PAWN_H

// end of pawn.h

