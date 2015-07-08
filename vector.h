
// vector.h

#ifndef VECTOR_H
#define VECTOR_H

// includes

#include "util.h"

// "constants"

const int_fast32_t IncNone = 0;
const int_fast32_t IncNb = 2 * 17 + 1;
const int_fast32_t IncOffset = 17;

const int_fast32_t DeltaNone = 0;
const int_fast32_t DeltaNb = 2 * 119 + 1;
const int_fast32_t DeltaOffset = 119;

// macros

#define DISTANCE(square_1,square_2) (Distance[DeltaOffset+((square_2)-(square_1))])

// variables

extern int_fast32_t Distance[DeltaNb];

// functions

extern void vector_init ();

extern bool delta_is_ok (int_fast32_t delta);
extern bool inc_is_ok   (int_fast32_t inc);

#endif // !defined VECTOR_H

// end of vector.h

