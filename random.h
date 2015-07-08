
// random.h

#ifndef RANDOM_H
#define RANDOM_H

// includes

#include "util.h"

// constants

const int_fast32_t RandomNb = 781;

// macros

#define RANDOM_64(n) (Random64[n])

// "constants"

extern const uint_fast64_t Random64[RandomNb];

// functions

extern void random_init ();

#endif // !defined RANDOM_H

// end of random.h

