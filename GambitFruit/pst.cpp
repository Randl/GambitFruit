// pst.cpp

// includes

#include "option.h"
#include "piece.h"
#include "pst.h"

// macros

#define P(piece_12, square_64, stage) (Pst[(piece_12)][(square_64)][(stage)])

// constants

static constexpr S32 A1 = 000, B1 = 001, C1 = 002, D1 = 003, E1 = 004, F1 = 005, G1 = 006, H1 = 007;
static constexpr S32 A2 = 010, B2 = 011, C2 = 012, D2 = 013, E2 = 014, F2 = 015, G2 = 016, H2 = 017;
static constexpr S32 A3 = 020, B3 = 021, C3 = 022, D3 = 023, E3 = 024, F3 = 025, G3 = 026, H3 = 027;
static constexpr S32 A4 = 030, B4 = 031, C4 = 032, D4 = 033, E4 = 034, F4 = 035, G4 = 036, H4 = 037;
static constexpr S32 A5 = 040, B5 = 041, C5 = 042, D5 = 043, E5 = 044, F5 = 045, G5 = 046, H5 = 047;
static constexpr S32 A6 = 050, B6 = 051, C6 = 052, D6 = 053, E6 = 054, F6 = 055, G6 = 056, H6 = 057;
static constexpr S32 A7 = 060, B7 = 061, C7 = 062, D7 = 063, E7 = 064, F7 = 065, G7 = 066, H7 = 067;
static constexpr S32 A8 = 070, B8 = 071, C8 = 072, D8 = 073, E8 = 074, F8 = 075, G8 = 076, H8 = 077;

/*
static constexpr S32 alt_pawn_opening[64] = {
	 0, 0, 0,  0,  0, 0, 0, 0,
	 1, 2, 3, -5, -5, 3, 2, 1,
	 3, 5, 8, 12, 12, 8, 5, 3,
	 5, 8,13, 26, 26,13, 8, 5,
	 7,11,18, 30, 30,18,11, 7,
	 9,14,23, 34, 34,23,14, 9,
	10,15,25, 35, 35,25,15,10,
	 0, 0, 0,  0,  0, 0, 0, 0
};*/

static constexpr std::array<S32, 64> alt_pawn_opening =
	{0, 0, 0, 0, 0, 0, 0, 0, -10, -4, 0, -5, -5, 0, -4, -10, -10, -4, 0, 8, 5, 0, -4, -10, -10, -4, 0, 16, 12, 0, -4,
	 -10, -10, -4, 0, 16, 12, 0, -4, -10, -10, -4, 0, 16, 12, 0, -4, -10, -10, -4, 0, 16, 12, 0, -4, -10, 0, 0, 0, 0, 0,
	 0, 0, 0};

static constexpr std::array<S32, 64> alt_pawn_endgame =
	{0, 0, 0, 0, 0, 0, 0, 0, -5, -2, 0, 0, 0, 0, -2, -5, -5, -2, 0, 3, 3, 0, -2, -5, -5, -2, 1, 7, 7, 1, -2, -5, -5, -2,
	 1, 7, 7, 1, -2, -5, -5, -2, 1, 7, 7, 1, -2, -5, -5, -2, 1, 7, 7, 1, -2, -5, 0, 0, 0, 0, 0, 0, 0, -5};

static constexpr std::array<S32, 64> alt_knight =
	{-20, 0, -10, -10, -10, -10, -10, -20, -10, 0, 0, 3, 3, 0, 0, -10, -10, 0, 5, 5, 5, 5, 0, -10, -10, 0, 5, 10, 10, 5,
	 0, -10, -10, 0, 5, 10, 10, 5, 0, -10, -10, 0, 5, 5, 5, 5, 0, -10, -10, 0, 0, 3, 3, 0, 0, -10, -20, -10, -10, -10,
	 -10, -10, -10, -20};

static constexpr std::array<S32, 64> alt_bishop =
	{-2, -2, -2, -2, -2, -2, -2, -2, -2, 8, 5, 5, 5, 5, 8, -2, -2, 3, 3, 5, 5, 3, 3, -2, -2, 2, 5, 4, 4, 5, 2, -2, -2,
	 2, 5, 4, 4, 5, 2, -2, -2, 3, 3, 5, 5, 3, 3, -2, -2, 8, 5, 5, 5, 5, 8, -2, -2, -2, -2, -2, -2, -2, -2, -2};

// constants and variables

static /* const */ S32 PieceActivityWeight = 256; // 100%
static constexpr S32 KingSafetyWeight = 256; // 100%
static /* const */ S32 PawnStructureWeight = 256; // 100%
static bool alt_pawn_table = false;
static bool alt_knight_table = false;
static bool alt_bishop_table = false;

static constexpr S8 PawnFileOpening = 5;
static constexpr S8 KnightCentreOpening = 5;
static constexpr S8 KnightCentreEndgame = 5;
static constexpr S8 KnightRankOpening = 5;
static constexpr S8 KnightBackRankOpening = 0;
static constexpr S16 KnightTrapped = 100;
static constexpr S8 BishopCentreOpening = 2;
static constexpr S8 BishopCentreEndgame = 3;
static constexpr S8 BishopBackRankOpening = 10;
static constexpr S8 BishopDiagonalOpening = 4;
static constexpr S8 RookFileOpening = 3;
static constexpr S8 QueenCentreOpening = 0;
static constexpr S8 QueenCentreEndgame = 4;
static constexpr S8 QueenBackRankOpening = 5;
static constexpr S8 KingCentreEndgame = 12;
static constexpr S8 KingFileOpening = 10;
static constexpr S8 KingRankOpening = 10;

// "constants"

static constexpr std::array<S8, 8> PawnFile = {-3, -1, +0, +1, +1, +0, -1, -3,};

static constexpr std::array<S8, 8> KnightLine = {-4, -2, +0, +1, +1, +0, -2, -4,};

static constexpr std::array<S8, 8> KnightRank = {-2, -1, +0, +1, +2, +3, +2, +1,};

static constexpr std::array<S8, 8> BishopLine = {-3, -1, +0, +1, +1, +0, -1, -3,};

static constexpr std::array<S8, 8> RookFile = {-2, -1, +0, +1, +1, +0, -1, -2,};

static constexpr std::array<S8, 8> QueenLine = {-3, -1, +0, +1, +1, +0, -1, -3,};

static constexpr std::array<S8, 8> KingLine = {-3, -1, +0, +1, +1, +0, -1, -3,};

static constexpr std::array<S8, 8> KingFile = {+3, +4, +2, +0, +0, +2, +4, +3,};

static constexpr std::array<S8, 8> KingRank = {+1, +0, -2, -3, -4, -5, -6, -7,};

// variables

std::array<std::array<std::array<S16, StageNb>, 64>, 12> Pst;

// prototypes

static S32 square_make(S32 file, S32 rank);

static S32 square_file(S32 square);
static S32 square_rank(S32 square);
static S32 square_opp(S32 square);

// functions

// pst_init()

void pst_init() {

	// UCI options

	PieceActivityWeight = (option_get_int("Piece Activity") * 256 + 50) / 100;
	//KingSafetyWeight    = (option_get_int("King Safety")    * 256 + 50) / 100;
	PawnStructureWeight = (option_get_int("Pawn Structure") * 256 + 50) / 100;
	alt_pawn_table = option_get_bool("Alt Pawn SQT");
	alt_knight_table = option_get_bool("Alt Knight SQT");
	alt_bishop_table = option_get_bool("Alt Bishop SQT");

	// init
	for (S32 piece = 0; piece < 12; ++piece)
		for (S32 sq = 0; sq < 64; ++sq)
			for (S32 stage = 0; stage < StageNb; ++stage)
				P(piece, sq, stage) = 0;

	// pawns
	S32 piece = WhitePawn12;

	if (alt_pawn_table) {
		for (S32 sq = 0; sq < 64; ++sq) {
			P(piece, sq, Opening) += alt_pawn_opening[sq];
			P(piece, sq, Endgame) += alt_pawn_endgame[sq];
		}
	} else {
		// file
		for (S32 sq = 0; sq < 64; ++sq)
			P(piece, sq, Opening) += PawnFile[square_file(sq)] * PawnFileOpening;

		// centre control

		P(piece, D3, Opening) += 10;
		P(piece, E3, Opening) += 10;

		P(piece, D4, Opening) += 20;
		P(piece, E4, Opening) += 20;

		P(piece, D5, Opening) += 10;
		P(piece, E5, Opening) += 10;
	}

	// weight
	for (S32 sq = 0; sq < 64; ++sq) {
		P(piece, sq, Opening) = (P(piece, sq, Opening) * PawnStructureWeight) / 256;
		P(piece, sq, Endgame) = (P(piece, sq, Endgame) * PawnStructureWeight) / 256;
	}

	// knights
	piece = WhiteKnight12;

	if (alt_knight_table) {
		for (S32 sq = 0; sq < 64; ++sq) {
			P(piece, sq, Opening) += alt_knight[sq];
			P(piece, sq, Endgame) += alt_knight[sq];
		}
	} else {
		// centre
		for (S32 sq = 0; sq < 64; ++sq) {
			P(piece, sq, Opening) += KnightLine[square_file(sq)] * KnightCentreOpening;
			P(piece, sq, Opening) += KnightLine[square_rank(sq)] * KnightCentreOpening;
			P(piece, sq, Endgame) += KnightLine[square_file(sq)] * KnightCentreEndgame;
			P(piece, sq, Endgame) += KnightLine[square_rank(sq)] * KnightCentreEndgame;
		}

		// rank
		for (S32 sq = 0; sq < 64; ++sq)
			P(piece, sq, Opening) += KnightRank[square_rank(sq)] * KnightRankOpening;

		// back rank

		for (S32 sq = A1; sq <= H1; ++sq) // HACK: only first rank
			P(piece, sq, Opening) -= KnightBackRankOpening;
	}

	// "trapped"
	P(piece, A8, Opening) -= KnightTrapped;
	P(piece, H8, Opening) -= KnightTrapped;

	// weight
	for (S32 sq = 0; sq < 64; ++sq) {
		P(piece, sq, Opening) = (P(piece, sq, Opening) * PieceActivityWeight) / 256;
		P(piece, sq, Endgame) = (P(piece, sq, Endgame) * PieceActivityWeight) / 256;
	}

	// bishops

	piece = WhiteBishop12;

	if (alt_bishop_table) {
		for (S32 sq = 0; sq < 64; ++sq) {
			P(piece, sq, Opening) += alt_bishop[sq];
			P(piece, sq, Endgame) += alt_bishop[sq];
		}
	} else {

		// centre
		for (S32 sq = 0; sq < 64; ++sq) {
			P(piece, sq, Opening) += BishopLine[square_file(sq)] * BishopCentreOpening;
			P(piece, sq, Opening) += BishopLine[square_rank(sq)] * BishopCentreOpening;
			P(piece, sq, Endgame) += BishopLine[square_file(sq)] * BishopCentreEndgame;
			P(piece, sq, Endgame) += BishopLine[square_rank(sq)] * BishopCentreEndgame;
		}

		// back rank
		for (S32 sq = A1; sq <= H1; ++sq) // HACK: only first rank
			P(piece, sq, Opening) -= BishopBackRankOpening;

		// main diagonals
		for (S32 i = 0; i < 8; ++i) {
			S32 sq = square_make(i, i);
			P(piece, sq, Opening) += BishopDiagonalOpening;
			P(piece, square_opp(sq), Opening) += BishopDiagonalOpening;
		}

		// weight
		for (S32 sq = 0; sq < 64; ++sq) {
			P(piece, sq, Opening) = (P(piece, sq, Opening) * PieceActivityWeight) / 256;
			P(piece, sq, Endgame) = (P(piece, sq, Endgame) * PieceActivityWeight) / 256;
		}
	}

	// rooks
	piece = WhiteRook12;

	// file
	for (S32 sq = 0; sq < 64; ++sq)
		P(piece, sq, Opening) += RookFile[square_file(sq)] * RookFileOpening;

	// weight
	for (S32 sq = 0; sq < 64; ++sq) {
		P(piece, sq, Opening) = (P(piece, sq, Opening) * PieceActivityWeight) / 256;
		P(piece, sq, Endgame) = (P(piece, sq, Endgame) * PieceActivityWeight) / 256;
	}

	// queens
	piece = WhiteQueen12;

	// centre
	for (S32 sq = 0; sq < 64; ++sq) {
		P(piece, sq, Opening) += QueenLine[square_file(sq)] * QueenCentreOpening;
		P(piece, sq, Opening) += QueenLine[square_rank(sq)] * QueenCentreOpening;
		P(piece, sq, Endgame) += QueenLine[square_file(sq)] * QueenCentreEndgame;
		P(piece, sq, Endgame) += QueenLine[square_rank(sq)] * QueenCentreEndgame;
	}

	// back rank
	for (S32 sq = A1; sq <= H1; ++sq) // HACK: only first rank
		P(piece, sq, Opening) -= QueenBackRankOpening;

	// weight
	for (S32 sq = 0; sq < 64; ++sq) {
		P(piece, sq, Opening) = (P(piece, sq, Opening) * PieceActivityWeight) / 256;
		P(piece, sq, Endgame) = (P(piece, sq, Endgame) * PieceActivityWeight) / 256;
	}

	// kings
	piece = WhiteKing12;

	// centre
	for (S32 sq = 0; sq < 64; ++sq) {
		P(piece, sq, Endgame) += KingLine[square_file(sq)] * KingCentreEndgame;
		P(piece, sq, Endgame) += KingLine[square_rank(sq)] * KingCentreEndgame;
	}

	// file
	for (S32 sq = 0; sq < 64; ++sq)
		P(piece, sq, Opening) += KingFile[square_file(sq)] * KingFileOpening;


	// rank
	for (S32 sq = 0; sq < 64; ++sq)
		P(piece, sq, Opening) += KingRank[square_rank(sq)] * KingRankOpening;

	// weight
	for (S32 sq = 0; sq < 64; ++sq) {
		P(piece, sq, Opening) = (P(piece, sq, Opening) * KingSafetyWeight) / 256;
		P(piece, sq, Endgame) = (P(piece, sq, Endgame) * PieceActivityWeight) / 256;
	}

	// symmetry copy for black
	for (S32 pc = 0; pc < 12; pc += 2) // HACK
		for (S32 sq = 0; sq < 64; ++sq)
			for (S32 stage = 0; stage < StageNb; ++stage)
				P(pc + 1, sq, stage) = -P(pc, square_opp(sq), stage); // HACK
}

// square_make()

static S32 square_make(S32 file, S32 rank) {

	ASSERT(file >= 0 && file < 8);
	ASSERT(rank >= 0 && rank < 8);

	return (rank << 3) | file;
}

// square_file()

static S32 square_file(S32 square) {

	ASSERT(square >= 0 && square < 64);
	return square & 7;
}

// square_rank()

static S32 square_rank(S32 square) {

	ASSERT(square >= 0 && square < 64);
	return square >> 3;
}

// square_opp()

static S32 square_opp(S32 square) { //inline?

	ASSERT(square >= 0 && square < 64);
	return square ^ 070;
}

// end of pst.cpp
