// attack.cpp

// includes

#include "attack.h"
#include "board.h"
#include "colour.h"
#include "move.h"
#include "piece.h"
#include "util.h"
#include "vector.h"

// variables

std::array<int_fast32_t, DeltaNb> DeltaIncLine;
std::array<int_fast32_t, DeltaNb> DeltaIncAll;

std::array<int_fast32_t, DeltaNb> DeltaMask;
std::array<int_fast32_t, IncNb> IncMask;

static std::array<int_fast32_t, PieceNb> PieceCode;

static std::array<std::array<int_fast32_t, 256>, 4> PieceDeltaSize; // 4kB
static std::array<std::array<std::array<int_fast32_t, 4>, 256>, 4> PieceDeltaDelta; // 16 kB

// prototypes

static void add_attack (int_fast32_t piece, int_fast32_t king, int_fast32_t target);

// functions

// attack_init()

void attack_init()  {

   // clear

	for (int_fast32_t delta = 0; delta < DeltaNb; ++delta)  {
		DeltaIncLine[delta] = IncNone;
		DeltaIncAll[delta] = IncNone;
		DeltaMask[delta] = 0;
	}

	for (int_fast32_t inc = 0; inc < IncNb; ++inc)
		IncMask[inc] = 0;


	// pawn attacks

	DeltaMask[DeltaOffset-17] |= BlackPawnFlag;
	DeltaMask[DeltaOffset-15] |= BlackPawnFlag;

	DeltaMask[DeltaOffset+15] |= WhitePawnFlag;
	DeltaMask[DeltaOffset+17] |= WhitePawnFlag;

	// knight attacks

	for (int_fast32_t dir = 0; dir < 8; ++dir) {

		const int_fast32_t delta = KnightInc[dir];
		ASSERT(delta_is_ok(delta));

		ASSERT(DeltaIncAll[DeltaOffset+delta]==IncNone);
		DeltaIncAll[DeltaOffset+delta] = delta;
		DeltaMask[DeltaOffset+delta] |= KnightFlag;
	}

	// bishop/queen attacks

	for (int_fast32_t dir = 0; dir < 4; ++dir) {

		const int_fast32_t inc = BishopInc[dir];
		ASSERT(inc!=IncNone);

		IncMask[IncOffset+inc] |= BishopFlag;

		for (int_fast32_t dist = 1; dist < 8; ++dist) {

			const int_fast32_t delta = inc*dist;
			ASSERT(delta_is_ok(delta));

			ASSERT(DeltaIncLine[DeltaOffset+delta]==IncNone);
			DeltaIncLine[DeltaOffset+delta] = inc;
			ASSERT(DeltaIncAll[DeltaOffset+delta]==IncNone);
			DeltaIncAll[DeltaOffset+delta] = inc;
			DeltaMask[DeltaOffset+delta] |= BishopFlag;
		}
	}

	// rook/queen attacks

	for (int_fast32_t dir = 0; dir < 4; ++dir) {

		const int_fast32_t inc = RookInc[dir];
		ASSERT(inc!=IncNone);

		IncMask[IncOffset+inc] |= RookFlag;

		for (int_fast32_t dist = 1; dist < 8; ++dist) {

			const int_fast32_t delta = inc*dist;
			ASSERT(delta_is_ok(delta));

			ASSERT(DeltaIncLine[DeltaOffset+delta]==IncNone);
			DeltaIncLine[DeltaOffset+delta] = inc;
			ASSERT(DeltaIncAll[DeltaOffset+delta]==IncNone);
			DeltaIncAll[DeltaOffset+delta] = inc;
			DeltaMask[DeltaOffset+delta] |= RookFlag;
		}
	}

	// king attacks

	for (int_fast32_t dir = 0; dir < 8; ++dir) {

		const int_fast32_t delta = KingInc[dir];
		ASSERT(delta_is_ok(delta));

		DeltaMask[DeltaOffset+delta] |= KingFlag;
	}

	// PieceCode[]

	for (int_fast32_t piece = 0; piece < PieceNb; ++piece)
		PieceCode[piece] = -1;

	PieceCode[WN] = 0;
	PieceCode[WB] = 1;
	PieceCode[WR] = 2;
	PieceCode[WQ] = 3;

	PieceCode[BN] = 0;
	PieceCode[BB] = 1;
	PieceCode[BR] = 2;
	PieceCode[BQ] = 3;

	// PieceDeltaSize[][] & PieceDeltaDelta[][][]

	for (int_fast32_t piece = 0; piece < 4; ++piece)
		for (int_fast32_t delta = 0; delta < 256; ++delta)
			PieceDeltaSize[piece][delta] = 0;


	for (int_fast32_t king = 0; king < SquareNb; ++king)
		if (SQUARE_IS_OK(king))
			for (int_fast32_t from = 0; from < SquareNb; ++from)
				if (SQUARE_IS_OK(from)) {

					int_fast32_t inc;
					// knight

					for (int_fast32_t pos = 0; (inc=KnightInc[pos]) != IncNone; ++pos) {
						const int_fast32_t to = from + inc;
						if (SQUARE_IS_OK(to) && DISTANCE(to,king) == 1)
							add_attack(0,king-from,to-from);
					}

					// bishop

					for (int_fast32_t pos = 0; (inc=BishopInc[pos]) != IncNone; ++pos)
						for (int_fast32_t to = from+inc; SQUARE_IS_OK(to); to += inc)
							if (DISTANCE(to,king) == 1) {
								add_attack(1,king-from,to-from);
								break;
							}

					// rook

					for (int_fast32_t pos = 0; (inc=RookInc[pos]) != IncNone; ++pos)
						for (int_fast32_t to = from+inc; SQUARE_IS_OK(to); to += inc)
							if (DISTANCE(to,king) == 1) {
								add_attack(2,king-from,to-from);
								break;
							}

					// queen

					for (int_fast32_t pos = 0; (inc=QueenInc[pos]) != IncNone; ++pos)
						for (int_fast32_t to = from+inc; SQUARE_IS_OK(to); to += inc)
							if (DISTANCE(to,king) == 1) {
								add_attack(3,king-from,to-from);
									break;
							}
            }

	for (int_fast32_t piece = 0; piece < 4; ++piece)
		for (int_fast32_t delta = 0; delta < 256; ++delta) {
			const int_fast32_t size = PieceDeltaSize[piece][delta];
			ASSERT(size>=0&&size<3);
			PieceDeltaDelta[piece][delta][size] = DeltaNone;
		}
}

// add_attack()

static void add_attack(int_fast32_t piece, int_fast32_t king, int_fast32_t target) {

	ASSERT(piece>=0&&piece<4);
	ASSERT(delta_is_ok(king));
	ASSERT(delta_is_ok(target));

	int_fast32_t size = PieceDeltaSize[piece][DeltaOffset+king];
	ASSERT(size>=0&&size<3);

	for (int_fast32_t i = 0; i < size; ++i)
		if (PieceDeltaDelta[piece][DeltaOffset+king][i] == target) return; // already in the table

	if (size < 2) {
		PieceDeltaDelta[piece][DeltaOffset+king][size] = target;
		++size;
		PieceDeltaSize[piece][DeltaOffset+king] = size;
	}
}

// is_attacked()

bool is_attacked(const board_t * board, int_fast32_t to, int_fast8_t colour) {

	ASSERT(board!=nullptr);
	ASSERT(SQUARE_IS_OK(to));
	ASSERT(COLOUR_IS_OK(colour));

	// pawn attack

	int_fast32_t inc = PAWN_MOVE_INC(colour);
	int_fast32_t pawn = PAWN_MAKE(colour);

	if (board->square[to-(inc-1)] == pawn) return true;
	if (board->square[to-(inc+1)] == pawn) return true;

	// piece attack
	for (auto from = board->piece[colour].begin(); from != board->piece[colour].end(); ++from) {

		const int_fast32_t piece = board->square[*from];
		const int_fast32_t delta = to - *from;

		if (PSEUDO_ATTACK(piece,delta)) {

			const int_fast32_t inc = DELTA_INC_ALL(delta);
			ASSERT(inc!=IncNone);

			int_fast32_t sq = *from;
			do {
				sq += inc;
				if (sq == to) return true;
			} while (board->square[sq] == Empty);
		}
	}

	return false;
}

// line_is_empty()

bool line_is_empty(const board_t * board, int_fast32_t from, int_fast32_t to) {

	ASSERT(board!=nullptr);
	ASSERT(SQUARE_IS_OK(from));
	ASSERT(SQUARE_IS_OK(to));

	const int_fast32_t delta = to - from;
	ASSERT(delta_is_ok(delta));

	const int_fast32_t inc = DELTA_INC_ALL(delta);
	ASSERT(inc!=IncNone);

	int_fast32_t sq = from;
	do {
		sq += inc;
		if (sq == to) return true;
	} while (board->square[sq] == Empty);

	return false; // blocker
}

// is_pinned()

bool is_pinned(const board_t * board, int_fast32_t square, int_fast8_t colour) {


	ASSERT(board!=nullptr);
	ASSERT(SQUARE_IS_OK(square));
	ASSERT(COLOUR_IS_OK(colour));

	const int_fast32_t from = square;
	const int_fast32_t to = KING_POS(board,colour);

	const int_fast32_t inc = DELTA_INC_LINE(to-from);
	if (inc == IncNone) return false; // not a line

	int_fast32_t sq = from;
	do sq += inc; while (board->square[sq] == Empty);

	if (sq != to) return false; // blocker

	sq = from;
	int_fast32_t piece;
	do sq -= inc; while ((piece=board->square[sq]) == Empty);

	return COLOUR_IS(piece,COLOUR_OPP(colour)) && SLIDER_ATTACK(piece,inc);
}

// attack_is_ok()

bool attack_is_ok(const attack_t * attack) {

	if (attack == nullptr) return false;

	// checks

	if (attack->dn < 0 || attack->dn > 2) return false;

	for (int_fast32_t i = 0; i < attack->dn; ++i) {
		if (!SQUARE_IS_OK(attack->ds[i])) return false;
		const int_fast32_t inc = attack->di[i];
		if (inc != IncNone && !inc_is_ok(inc)) return false;
	}

	if (attack->ds[attack->dn] != SquareNone) return false;
	if (attack->di[attack->dn] != IncNone) return false;

	return true;
}

// attack_set()

void attack_set(attack_t * attack, const board_t * board) {

	ASSERT(attack!=nullptr);
	ASSERT(board!=nullptr);

	// init

	attack->dn = 0;

	const int_fast32_t me = board->turn;
	const int_fast32_t opp = COLOUR_OPP(me);

	int_fast32_t to = KING_POS(board,me);

	// pawn attacks
	{
		int_fast32_t inc = PAWN_MOVE_INC(opp);
		int_fast32_t pawn = PAWN_MAKE(opp);

		int_fast32_t from = to - (inc-1);
		if (board->square[from] == pawn) {
			attack->ds[attack->dn] = from;
			attack->di[attack->dn] = IncNone;
			attack->dn++;
   	}

   	from = to - (inc+1);
   	if (board->square[from] == pawn) {
   		attack->ds[attack->dn] = from;
   		attack->di[attack->dn] = IncNone;
   		attack->dn++;
   	}
	}
    // piece attacks
	for (auto from = board->piece[opp].begin() + 1; from != board->piece[opp].end(); ++from) { // HACK: no king

		const int_fast32_t piece = board->square[*from];

		const int_fast32_t delta = to - *from;
		ASSERT(delta_is_ok(delta));

		if (PSEUDO_ATTACK(piece,delta)) {

			int_fast32_t inc = IncNone;

			if (PIECE_IS_SLIDER(piece)) {

				// check for blockers

				inc = DELTA_INC_LINE(delta);
				ASSERT(inc!=IncNone);

            	int_fast32_t sq = *from;
            	do sq += inc; while (board->square[sq] == Empty);

            	if (sq != to) continue; // blocker => next attacker
			}

			attack->ds[attack->dn] = *from;
			attack->di[attack->dn] = -inc; // HACK
			attack->dn++;
		}
	}

	attack->ds[attack->dn] = SquareNone;
	attack->di[attack->dn] = IncNone;

	// debug
	ASSERT(attack_is_ok(attack));
}

// piece_attack_king()

bool piece_attack_king(const board_t * board, int_fast32_t piece, int_fast32_t from, int_fast32_t king) {

	ASSERT(board!=nullptr);
	ASSERT(piece_is_ok(piece));
	ASSERT(SQUARE_IS_OK(from));
	ASSERT(SQUARE_IS_OK(king));

	const int_fast32_t code = PieceCode[piece];
	ASSERT(code>=0&&code<4);

	if (PIECE_IS_SLIDER(piece)) {
		int_fast32_t delta;
		for (auto delta_ptr = PieceDeltaDelta[code][DeltaOffset+(king-from)].begin(); (delta=*delta_ptr) != DeltaNone; ++delta_ptr) {

			ASSERT(delta_is_ok(delta));

			const int_fast32_t inc = DeltaIncLine[DeltaOffset+delta];
			ASSERT(inc!=IncNone);

			const int_fast32_t to = from + delta;

			int_fast32_t sq = from;
			do {
				sq += inc;
				if (sq == to && SQUARE_IS_OK(to)) {
					ASSERT(DISTANCE(to,king)==1);
					return true;
				}
			} while (board->square[sq] == Empty);
		}

	} else { // non-slider

		int_fast32_t delta;
		for (auto delta_ptr = PieceDeltaDelta[code][DeltaOffset+(king-from)].begin(); (delta=*delta_ptr) != DeltaNone; ++delta_ptr) {

			ASSERT(delta_is_ok(delta));

			const int_fast32_t to = from + delta;

			if (SQUARE_IS_OK(to)) {
				ASSERT(DISTANCE(to,king)==1);
				return true;
			}
		}
	}

	return false;
}

// end of attack.cpp
