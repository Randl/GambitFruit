
// value.cpp

// includes

#include "piece.h"
#include "search.h"
#include "util.h"
#include "value.h"

// variables

std::array<int_fast32_t, PieceNb> ValuePiece;

// functions

// value_init()

void value_init() {

   int_fast32_t piece;

   // ValuePiece[]

   for (piece = 0; piece < PieceNb; ++piece) ValuePiece[piece] = -1;

   ValuePiece[Empty] = 0; // needed?
   ValuePiece[Edge]  = 0; // needed?

   ValuePiece[WP] = ValuePawn;
   ValuePiece[WN] = ValueKnight;
   ValuePiece[WB] = ValueBishop;
   ValuePiece[WR] = ValueRook;
   ValuePiece[WQ] = ValueQueen;
   ValuePiece[WK] = ValueKing;

   ValuePiece[BP] = ValuePawn;
   ValuePiece[BN] = ValueKnight;
   ValuePiece[BB] = ValueBishop;
   ValuePiece[BR] = ValueRook;
   ValuePiece[BQ] = ValueQueen;
   ValuePiece[BK] = ValueKing;
}

// value_is_ok()

bool value_is_ok(int_fast32_t value) {

   if (value < -ValueInf || value > +ValueInf) return false;

   return true;
}

// range_is_ok()

bool range_is_ok(int_fast32_t min, int_fast32_t max) {

   if (!value_is_ok(min)) return false;
   if (!value_is_ok(max)) return false;

   if (min >= max) return false; // alpha-beta-like ranges cannot be null

   return true;
}

// value_is_mate()

bool value_is_mate(int_fast32_t value) {

   ASSERT(value_is_ok(value));

   if (value < -ValueEvalInf || value > +ValueEvalInf) return true;

   return false;
}

// value_to_trans()

int_fast32_t value_to_trans(int_fast32_t value, int_fast32_t height) {

   ASSERT(value_is_ok(value));
   ASSERT(height_is_ok(height));

   if (value < -ValueEvalInf) {
      value -= height;
   } else if (value > +ValueEvalInf) {
      value += height;
   }

   ASSERT(value_is_ok(value));

   return value;
}

// value_from_trans()

int_fast32_t value_from_trans(int_fast32_t value, int_fast32_t height) {

   ASSERT(value_is_ok(value));
   ASSERT(height_is_ok(height));

   if (value < -ValueEvalInf) {
      value += height;
   } else if (value > +ValueEvalInf) {
      value -= height;
   }

   ASSERT(value_is_ok(value));

   return value;
}

// value_to_mate()

int_fast32_t value_to_mate(int_fast32_t value) {

   int_fast32_t dist;

   ASSERT(value_is_ok(value));

   if (value < -ValueEvalInf) {

      dist = (ValueMate + value) / 2;
      ASSERT(dist>0);

      return -dist;

   } else if (value > +ValueEvalInf) {

      dist = (ValueMate - value + 1) / 2;
      ASSERT(dist>0);

      return +dist;
   }

   return 0;
}

// end of value.cpp

