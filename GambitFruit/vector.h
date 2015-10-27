// vector.h

#ifndef VECTOR_H
#define VECTOR_H

// includes

#include <array>
#include "util.h"

// "constants"

constexpr S8 IncNone = 0;
constexpr S8 IncNb = 2 * 17 + 1;
constexpr S8 IncOffset = 17;

constexpr S8 DeltaNone = 0;
constexpr S16 DeltaNb = 2 * 119 + 1;
constexpr S16 DeltaOffset = 119;

// macros

#define DISTANCE(square_1, square_2) (Distance[DeltaOffset+((square_2)-(square_1))])

// variables

extern std::array<S32, DeltaNb> Distance;

// functions

extern void vector_init();

extern bool delta_is_ok(S32 delta);
extern bool inc_is_ok(S32 inc);

#endif // !defined VECTOR_H

// end of vector.h

