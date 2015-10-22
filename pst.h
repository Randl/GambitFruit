// pst.h

#ifndef PST_H
#define PST_H

// includes

#include <array>
#include "util.h"

// constants

constexpr S8 Opening = 0;
constexpr S8 Endgame = 1;
constexpr S8 StageNb = 2;

// macros

#define PST(piece_12, square_64, stage) (Pst[piece_12][square_64][stage])

// variables

extern std::array<std::array<std::array<S16, StageNb>, 64>, 12> Pst;

// functions

extern void pst_init();

#endif // !defined PST_H

// end of pst.h

