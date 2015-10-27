// value.cpp

// includes

#include "piece.h"
#include "search.h"
#include "value.h"

// variables

std::array<S32, PieceNb> ValuePiece;

// functions

// value_init()

void value_init() {

	// ValuePiece[]
	for (S32 piece = 0; piece < PieceNb; ++piece)
		ValuePiece[piece] = -1;

	ValuePiece[Empty] = 0; // needed?
	ValuePiece[Edge] = 0; // needed?

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

bool value_is_ok(S32 value) {

	if (value < -ValueInf || value > +ValueInf) return false;

	return true;
}

// range_is_ok()

bool range_is_ok(S32 min, S32 max) {

	if (!value_is_ok(min)) return false;
	if (!value_is_ok(max)) return false;

	return min < max; // alpha-beta-like ranges cannot be null
}

// value_is_mate()

bool value_is_mate(S32 value) {

	ASSERT(value_is_ok(value));

	if (value < -ValueEvalInf || value > +ValueEvalInf) return true;

	return false;
}

// value_to_trans()

S32 value_to_trans(S32 value, S32 height) {

	ASSERT(value_is_ok(value));
	ASSERT(height_is_ok(height));

	if (value < -ValueEvalInf)
		value -= height;
	else if (value > +ValueEvalInf)
		value += height;

	ASSERT(value_is_ok(value));

	return value;
}

// value_from_trans()

S32 value_from_trans(S32 value, S32 height) {

	ASSERT(value_is_ok(value));
	ASSERT(height_is_ok(height));

	if (value < -ValueEvalInf)
		value += height;
	else if (value > +ValueEvalInf)
		value -= height;

	ASSERT(value_is_ok(value));
	return value;
}

// value_to_mate()

S32 value_to_mate(S32 value) {

	ASSERT(value_is_ok(value));

	if (value < -ValueEvalInf) {
		const S32 dist = (ValueMate + value) / 2;
		ASSERT(dist > 0);
		return -dist;
	} else if (value > +ValueEvalInf) {
		const S32 dist = (ValueMate - value + 1) / 2;
		ASSERT(dist > 0);
		return +dist;
	}
	return 0;
}

// end of value.cpp
