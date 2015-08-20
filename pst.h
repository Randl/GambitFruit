// pst.h

#ifndef PST_H
#define PST_H

// includes

#include <array>
#include "util.h"

// constants

constexpr int_fast8_t Opening = 0;
constexpr int_fast8_t Endgame = 1;
constexpr int_fast8_t StageNb = 2;

// macros

#define PST(piece_12, square_64, stage) (Pst[piece_12][square_64][stage])

// variables

extern std::array<std::array<std::array<int_fast16_t, StageNb>, 64>, 12> Pst;

// functions

extern void pst_init();

#endif // !defined PST_H

// end of pst.h

