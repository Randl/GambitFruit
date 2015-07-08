
// pst.h

#ifndef PST_H
#define PST_H

// includes

#include "util.h"

// constants

const int_fast32_t Opening = 0;
const int_fast32_t Endgame = 1;
const int_fast32_t StageNb = 2;

// macros

#define PST(piece_12,square_64,stage) (Pst[piece_12][square_64][stage])

// variables

extern int_fast16_t Pst[12][64][StageNb];

// functions

extern void pst_init ();

#endif // !defined PST_H

// end of pst.h

