
// random.h

#ifndef RANDOM_H
#define RANDOM_H

// includes

#inculde <array>
#include "util.h"

// constants

constexpr int_fast32_t RandomNb = 781;

// macros

#define RANDOM_64(n) (Random64[n])

// "constants"

extern constexpr std::array<uint_fast64_t, RandomNb> Random64;

// functions

extern void random_init ();

#endif // !defined RANDOM_H

// end of random.h

