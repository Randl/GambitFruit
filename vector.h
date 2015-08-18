
// vector.h

#ifndef VECTOR_H
#define VECTOR_H

// includes

#include <array>
#include "util.h"

// "constants"

constexpr int_fast8_t IncNone = 0;
constexpr int_fast8_t IncNb = 2 * 17 + 1;
constexpr int_fast8_t IncOffset = 17;

constexpr int_fast8_t DeltaNone = 0;
constexpr int_fast16_t DeltaNb = 2 * 119 + 1;
constexpr int_fast16_t DeltaOffset = 119;

// macros

#define DISTANCE(square_1,square_2) (Distance[DeltaOffset+((square_2)-(square_1))])

// variables

extern std::array<int_fast32_t, DeltaNb> Distance;

// functions

extern void vector_init ();

extern bool delta_is_ok (int_fast32_t delta);
extern bool inc_is_ok   (int_fast32_t inc);

#endif // !defined VECTOR_H

// end of vector.h

