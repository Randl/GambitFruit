
// value.h

#ifndef VALUE_H
#define VALUE_H

// includes

#include "piece.h"
#include "util.h"

// constants

const int_fast32_t ValuePawn   = 100;   // was 100
const int_fast32_t ValueKnight = 325;   // was 300
const int_fast32_t ValueBishop = 325;   // was 300
const int_fast32_t ValueRook   = 500;   // was 500
const int_fast32_t ValueQueen  = 1000;  // was 900
const int_fast32_t ValueKing   = 10000; // was 10000

const int_fast32_t ValueNone    = -32767;
/*const*/ extern int_fast32_t ValueDraw;
const int_fast32_t ValueMate    = 30000;
const int_fast32_t ValueInf     = ValueMate;
const int_fast32_t ValueEvalInf = ValueMate - 256; // handle mates upto 255 plies

// macros

#define VALUE_MATE(height) (-ValueMate+(height))
#define VALUE_PIECE(piece) (ValuePiece[piece])

// variables

extern int_fast32_t ValuePiece[PieceNb];

// functions

extern void value_init       ();

extern bool value_is_ok      (int_fast32_t value);
extern bool range_is_ok      (int_fast32_t min, int_fast32_t max);

extern bool value_is_mate    (int_fast32_t value);

extern int_fast32_t  value_to_trans   (int_fast32_t value, int_fast32_t height);
extern int_fast32_t  value_from_trans (int_fast32_t value, int_fast32_t height);

extern int_fast32_t  value_to_mate    (int_fast32_t value);

#endif // !defined VALUE_H

// end of value.h

