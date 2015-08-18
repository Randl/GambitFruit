
// colour.h

#ifndef COLOUR_H
#define COLOUR_H

// includes

#include <cstdint>
#include "util.h"

// constants

constexpr int_fast8_t ColourNone = -1;
constexpr int_fast8_t White = 0;
constexpr int_fast8_t Black = 1;
constexpr int_fast8_t ColourNb = 2;

constexpr int_fast8_t WhiteFlag = 1 << White;
constexpr int_fast8_t BlackFlag = 1 << Black;

// macros

#define COLOUR_IS_OK(colour)    (((colour)&~1)==0)

#define COLOUR_IS_WHITE(colour) ((colour)==White)
#define COLOUR_IS_BLACK(colour) ((colour)!=White)

#define COLOUR_IS(piece,colour) (FLAG_IS((piece),COLOUR_FLAG(colour)))
#define FLAG_IS(piece,flag)     (((piece)&(flag))!=0)

#define COLOUR_OPP(colour)      ((colour)^(White^Black))
#define COLOUR_FLAG(colour)     ((colour)+1)

#endif // !defined COLOUR_H

// end of colour.h
