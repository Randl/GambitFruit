// piece.cpp

// includes

#include <cstring>

#include "colour.h"
#include "piece.h"

// "constants"

static const char PieceString[12 + 1] = "PpNnBbRrQqKk";

// variables

std::array<S32, PieceNb> PieceTo12;
std::array<S32, PieceNb> PieceOrder;

std::array<const inc_t *, PieceNb> PieceInc;

// functions

// piece_init()

void piece_init() {
	// PieceTo12[]

	for (U16 piece = 0; piece < PieceNb; ++piece)
		PieceTo12[piece] = -1;

	for (U8 piece_12 = 0; piece_12 < 12; ++piece_12)
		PieceTo12[PieceFrom12[piece_12]] = piece_12;


	// PieceOrder[]

	for (U16 piece = 0; piece < PieceNb; ++piece)
		PieceOrder[piece] = -1;

	for (U8 piece_12 = 0; piece_12 < 12; ++piece_12)
		PieceOrder[PieceFrom12[piece_12]] = piece_12 >> 1;

	// PieceInc[]

	for (U16 piece = 0; piece < PieceNb; ++piece)
		PieceInc[piece] = nullptr;

	PieceInc[WhiteKnight256] = KnightInc.data();
	PieceInc[WhiteBishop256] = BishopInc.data();
	PieceInc[WhiteRook256] = RookInc.data();
	PieceInc[WhiteQueen256] = QueenInc.data();
	PieceInc[WhiteKing256] = KingInc.data();

	PieceInc[BlackKnight256] = KnightInc.data();
	PieceInc[BlackBishop256] = BishopInc.data();
	PieceInc[BlackRook256] = RookInc.data();
	PieceInc[BlackQueen256] = QueenInc.data();
	PieceInc[BlackKing256] = KingInc.data();
}

// piece_is_ok()

bool piece_is_ok(S32 piece) { //TODO: piece is U16

	if (piece < 0 || piece >= PieceNb) return false;
	if (PieceTo12[piece] < 0) return false;

	return true;
}

// piece_from_12()

S32 piece_from_12(S32 piece_12) { //TODO: piece12 -> U8

	ASSERT(piece_12 >= 0 && piece_12 < 12);

	return PieceFrom12[piece_12];
}

// piece_to_char()

S32 piece_to_char(S32 piece) {

	ASSERT(piece_is_ok(piece));

	return PieceString[PIECE_TO_12(piece)];
}

// piece_from_char()

S32 piece_from_char(S32 c) {

	const char *ptr = strchr(PieceString, c);
	if (ptr == nullptr) return PieceNone256;

	return piece_from_12(ptr - PieceString);
}

// end of piece.cpp
