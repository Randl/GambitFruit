// value.h

#ifndef VALUE_H
#define VALUE_H

// includes

#include <array>
#include "piece.h"
#include "util.h"

// constants

constexpr S16 ValuePawn = 100;   // was 100
constexpr S16 ValueKnight = 325;   // was 300
constexpr S16 ValueBishop = 325;   // was 300
constexpr S16 ValueRook = 500;   // was 500
constexpr S16 ValueQueen = 1000;  // was 900
constexpr S16 ValueKing = 10000; // was 10000

constexpr S32 ValueNone = -32767;
/*const*/ extern S32 ValueDraw;
constexpr S32 ValueMate = 30000;
constexpr S32 ValueInf = ValueMate;
constexpr S32 ValueEvalInf = ValueMate - 256; // handle mates upto 255 plies

// macros

#define VALUE_MATE(height) (-ValueMate+(height))
#define VALUE_PIECE(piece) (ValuePiece[piece])

// variables

extern std::array<S32, PieceNb> ValuePiece;

// functions

extern void value_init();

extern bool value_is_ok(S32 value);
extern bool range_is_ok(S32 min, S32 max);

extern bool value_is_mate(S32 value);

extern S32 value_to_trans(S32 value, S32 height);
extern S32 value_from_trans(S32 value, S32 height);

extern S32 value_to_mate(S32 value);

#endif // !defined VALUE_H

// end of value.h

