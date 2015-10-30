// square.cpp

// includes

#include "colour.h"
#include "square.h"

// variables

std::array<S32, SquareNb> SquareTo64;
std::array<bool, SquareNb> SquareIsPromote;

// functions

// square_init()

void square_init() {

	// SquareTo64[]
	for (U16 sq = 0; sq < SquareNb; ++sq)
		SquareTo64[sq] = -1;

	for (U8 sq = 0; sq < 64; ++sq)
		SquareTo64[SquareFrom64[sq]] = sq;

	// SquareIsPromote[]
	for (U16 sq = 0; sq < SquareNb; ++sq)
		SquareIsPromote[sq] = SQUARE_IS_OK(sq) && (SQUARE_RANK(sq) == Rank1 || SQUARE_RANK(sq) == Rank8);
}

// file_from_char()

S32 file_from_char(S32 c) {
	ASSERT(c >= 'a' && c <= 'h');
	return FileA + (c - 'a');
}

// rank_from_char()

S32 rank_from_char(S32 c) {
	ASSERT(c >= '1' && c <= '8');
	return Rank1 + (c - '1');
}

// file_to_char()

char file_to_char(S32 file) {
	ASSERT(file >= FileA && file <= FileH);
	return 'a' + (file - FileA);
}

// rank_to_char()

char rank_to_char(S32 rank) {
	ASSERT(rank >= Rank1 && rank <= Rank8);
	return '1' + (rank - Rank1);
}

// square_to_string()

bool square_to_string(S32 square, char string[], S32 size) {
	ASSERT(SQUARE_IS_OK(square));
	ASSERT(string != nullptr);
	ASSERT(size >= 3);

	if (size < 3) return false;

	string[0] = file_to_char(SQUARE_FILE(square));
	string[1] = rank_to_char(SQUARE_RANK(square));
	string[2] = '\0';

	return true;
}

// square_from_string()

S32 square_from_string(const char string[]) {

	ASSERT(string != nullptr);

	if (string[0] < 'a' || string[0] > 'h') return SquareNone;
	if (string[1] < '1' || string[1] > '8') return SquareNone;
	if (string[2] != '\0') return SquareNone;

	const S32 file = file_from_char(string[0]), rank = rank_from_char(string[1]);
	return SQUARE_MAKE(file, rank);
}

// end of square.cpp
