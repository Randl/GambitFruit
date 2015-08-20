// value.h

#ifndef VALUE_H
#define VALUE_H

// includes

#include <array>
#include "piece.h"
#include "util.h"

// constants

constexpr int_fast16_t ValuePawn   = 100;   // was 100
constexpr int_fast16_t ValueKnight = 325;   // was 300
constexpr int_fast16_t ValueBishop = 325;   // was 300
constexpr int_fast16_t ValueRook   = 500;   // was 500
constexpr int_fast16_t ValueQueen  = 1000;  // was 900
constexpr int_fast16_t ValueKing   = 10000; // was 10000

constexpr int_fast32_t ValueNone    = -32767;
/*const*/ extern int_fast32_t ValueDraw;
constexpr int_fast32_t ValueMate    = 30000;
constexpr int_fast32_t ValueInf     = ValueMate;
constexpr int_fast32_t ValueEvalInf = ValueMate - 256; // handle mates upto 255 plies

// macros

#define VALUE_MATE(height) (-ValueMate+(height))
#define VALUE_PIECE(piece) (ValuePiece[piece])

// variables

extern std::array<int_fast32_t, PieceNb> ValuePiece;

// functions

extern void value_init();

extern bool value_is_ok(int_fast32_t value);
extern bool range_is_ok(int_fast32_t min, int_fast32_t max);

extern bool value_is_mate(int_fast32_t value);

extern int_fast32_t value_to_trans(int_fast32_t value, int_fast32_t height);
extern int_fast32_t value_from_trans(int_fast32_t value, int_fast32_t height);

extern int_fast32_t value_to_mate(int_fast32_t value);

#endif // !defined VALUE_H

// end of value.h

