// attack.cpp

// includes

#include "attack.h"

// variables

std::array<S32, DeltaNb> DeltaIncLine;
std::array<S32, DeltaNb> DeltaIncAll;

std::array<S32, DeltaNb> DeltaMask;
std::array<S32, IncNb> IncMask;

static std::array<S32, PieceNb> PieceCode;

static std::array<std::array<S32, 256>, 4> PieceDeltaSize; // 4kB
static std::array<std::array<std::array<S32, 4>, 256>, 4> PieceDeltaDelta; // 16 kB

// prototypes

static void add_attack(S32 piece, S32 king, S32 target);

// functions

// attack_init()

void attack_init() {

	// clear

	for (S16 delta = 0; delta < DeltaNb; ++delta) {
		DeltaIncLine[delta] = IncNone;
		DeltaIncAll[delta] = IncNone;
		DeltaMask[delta] = 0;
	}

	for (S8 inc = 0; inc < IncNb; ++inc)
		IncMask[inc] = 0;


	// pawn attacks

	DeltaMask[DeltaOffset - 17] |= BlackPawnFlag;
	DeltaMask[DeltaOffset - 15] |= BlackPawnFlag;

	DeltaMask[DeltaOffset + 15] |= WhitePawnFlag;
	DeltaMask[DeltaOffset + 17] |= WhitePawnFlag;

	// knight attacks

	for (U8 dir = 0; dir < 8; ++dir) {

		const S32 delta = KnightInc[dir];
		ASSERT(delta_is_ok(delta));

		ASSERT(DeltaIncAll[DeltaOffset + delta] == IncNone);
		DeltaIncAll[DeltaOffset + delta] = delta;
		DeltaMask[DeltaOffset + delta] |= KnightFlag;
	}

	// bishop/queen attacks

	for (U8 dir = 0; dir < 4; ++dir) {

		const S32 inc = BishopInc[dir];
		ASSERT(inc != IncNone);

		IncMask[IncOffset + inc] |= BishopFlag;

		for (U8 dist = 1; dist < 8; ++dist) {

			const S32 delta = inc * dist;
			ASSERT(delta_is_ok(delta));

			ASSERT(DeltaIncLine[DeltaOffset + delta] == IncNone);
			DeltaIncLine[DeltaOffset + delta] = inc;
			ASSERT(DeltaIncAll[DeltaOffset + delta] == IncNone);
			DeltaIncAll[DeltaOffset + delta] = inc;
			DeltaMask[DeltaOffset + delta] |= BishopFlag;
		}
	}

	// rook/queen attacks

	for (U8 dir = 0; dir < 4; ++dir) {

		const S32 inc = RookInc[dir];
		ASSERT(inc != IncNone);

		IncMask[IncOffset + inc] |= RookFlag;

		for (U8 dist = 1; dist < 8; ++dist) {

			const S32 delta = inc * dist;
			ASSERT(delta_is_ok(delta));

			ASSERT(DeltaIncLine[DeltaOffset + delta] == IncNone);
			DeltaIncLine[DeltaOffset + delta] = inc;
			ASSERT(DeltaIncAll[DeltaOffset + delta] == IncNone);
			DeltaIncAll[DeltaOffset + delta] = inc;
			DeltaMask[DeltaOffset + delta] |= RookFlag;
		}
	}

	// king attacks

	for (U8 dir = 0; dir < 8; ++dir) {

		const S32 delta = KingInc[dir];
		ASSERT(delta_is_ok(delta));

		DeltaMask[DeltaOffset + delta] |= KingFlag;
	}

	// PieceCode[]

	for (U16 piece = 0; piece < PieceNb; ++piece)
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

	for (U8 piece = 0; piece < 4; ++piece)
		for (U16 delta = 0; delta < 256; ++delta)
			PieceDeltaSize[piece][delta] = 0;

	for (U16 king = 0; king < SquareNb; ++king)
		if (SQUARE_IS_OK(king))
			for (U16 from = 0; from < SquareNb; ++from)
				if (SQUARE_IS_OK(from)) {

					S32 inc;
					// knight

					for (S32 pos = 0; (inc = KnightInc[pos]) != IncNone; ++pos) {
						const S32 to = from + inc;
						if (SQUARE_IS_OK(to) && DISTANCE(to, king) == 1)
							add_attack(0, king - from, to - from);
					}

					// bishop

					for (S32 pos = 0; (inc = BishopInc[pos]) != IncNone; ++pos)
						for (S32 to = from + inc; SQUARE_IS_OK(to); to += inc)
							if (DISTANCE(to, king) == 1) {
								add_attack(1, king - from, to - from);
								break;
							}

					// rook

					for (S32 pos = 0; (inc = RookInc[pos]) != IncNone; ++pos)
						for (S32 to = from + inc; SQUARE_IS_OK(to); to += inc)
							if (DISTANCE(to, king) == 1) {
								add_attack(2, king - from, to - from);
								break;
							}

					// queen

					for (S32 pos = 0; (inc = QueenInc[pos]) != IncNone; ++pos)
						for (S32 to = from + inc; SQUARE_IS_OK(to); to += inc)
							if (DISTANCE(to, king) == 1) {
								add_attack(3, king - from, to - from);
								break;
							}
				}

	for (U8 piece = 0; piece < 4; ++piece)
		for (U16 delta = 0; delta < 256; ++delta) {
			const S32 size = PieceDeltaSize[piece][delta];
			ASSERT(size >= 0 && size < 3);
			PieceDeltaDelta[piece][delta][size] = DeltaNone;
		}
}

// add_attack()

static void add_attack(S32 piece, S32 king, S32 target) {

	ASSERT(piece >= 0 && piece < 4);
	ASSERT(delta_is_ok(king));
	ASSERT(delta_is_ok(target));

	S32 size = PieceDeltaSize[piece][DeltaOffset + king];
	ASSERT(size >= 0 && size < 3);

	for (S32 i = 0; i < size; ++i)
		if (PieceDeltaDelta[piece][DeltaOffset + king][i] == target) return; // already in the table

	if (size < 2) {
		PieceDeltaDelta[piece][DeltaOffset + king][size] = target;
		++size;
		PieceDeltaSize[piece][DeltaOffset + king] = size;
	}
}

// is_attacked()

bool is_attacked(const board_t *board, S32 to, S8 colour) {

	ASSERT(board != nullptr);
	ASSERT(SQUARE_IS_OK(to));
	ASSERT(COLOUR_IS_OK(colour));

	// pawn attack

	S32 inc = PAWN_MOVE_INC(colour);
	S32 pawn = PAWN_MAKE(colour);

	if (board->square[to - (inc - 1)] == pawn) return true;
	if (board->square[to - (inc + 1)] == pawn) return true;

	// piece attack
	for (auto from = board->piece[colour].begin(); from != board->piece[colour].end(); ++from) {

		const S32 piece = board->square[*from];
		const S32 delta = to - *from;

		if (PSEUDO_ATTACK(piece, delta)) {

			const S32 inc_piece = DELTA_INC_ALL(delta);
			ASSERT(inc_piece != IncNone);

			S32 sq = *from;
			do {
				sq += inc_piece;
				if (sq == to) return true;
			} while (board->square[sq] == Empty);
		}
	}

	return false;
}

// line_is_empty()

bool line_is_empty(const board_t *board, S32 from, S32 to) {

	ASSERT(board != nullptr);
	ASSERT(SQUARE_IS_OK(from));
	ASSERT(SQUARE_IS_OK(to));

	const S32 delta = to - from;
	ASSERT(delta_is_ok(delta));

	const S32 inc = DELTA_INC_ALL(delta);
	ASSERT(inc != IncNone);

	S32 sq = from;
	do {
		sq += inc;
		if (sq == to) return true;
	} while (board->square[sq] == Empty);

	return false; // blocker
}

// is_pinned()

bool is_pinned(const board_t *board, S32 square, S8 colour) {

	ASSERT(board != nullptr);
	ASSERT(SQUARE_IS_OK(square));
	ASSERT(COLOUR_IS_OK(colour));

	const S32 from = square;
	const S32 to = KING_POS(board, colour);

	const S32 inc = DELTA_INC_LINE(to - from);
	if (inc == IncNone) return false; // not a line

	S32 sq = from;
	do sq += inc; while (board->square[sq] == Empty);

	if (sq != to) return false; // blocker

	sq = from;
	S32 piece;
	do sq -= inc; while ((piece = board->square[sq]) == Empty);

	return COLOUR_IS(piece, COLOUR_OPP(colour)) && SLIDER_ATTACK(piece, inc);
}

// attack_is_ok()

bool attack_is_ok(const attack_t *attack) {

	if (attack == nullptr) return false;

	// checks

	if (attack->dn < 0 || attack->dn > 2) return false;

	for (S32 i = 0; i < attack->dn; ++i) {
		if (!SQUARE_IS_OK(attack->ds[i])) return false;
		const S32 inc = attack->di[i];
		if (inc != IncNone && !inc_is_ok(inc)) return false;
	}

	if (attack->ds[attack->dn] != SquareNone) return false;
	if (attack->di[attack->dn] != IncNone) return false;

	return true;
}

// attack_set()

void attack_set(attack_t *attack, const board_t *board) {

	ASSERT(attack != nullptr);
	ASSERT(board != nullptr);

	// init

	attack->dn = 0;

	const S8 me = board->turn, opp = COLOUR_OPP(me);
	S32 to = KING_POS(board, me);

	// pawn attacks
	{
		S32 inc = PAWN_MOVE_INC(opp);
		S32 pawn = PAWN_MAKE(opp);

		S32 from = to - (inc - 1);
		if (board->square[from] == pawn) {
			attack->ds[attack->dn] = from;
			attack->di[attack->dn] = IncNone;
			attack->dn++;
		}

		from = to - (inc + 1);
		if (board->square[from] == pawn) {
			attack->ds[attack->dn] = from;
			attack->di[attack->dn] = IncNone;
			attack->dn++;
		}
	}
	// piece attacks
	for (auto from = board->piece[opp].begin() + 1; from != board->piece[opp].end(); ++from) { // HACK: no king

		const S32 piece = board->square[*from];

		const S32 delta = to - *from;
		ASSERT(delta_is_ok(delta));

		if (PSEUDO_ATTACK(piece, delta)) {

			S32 inc = IncNone;

			if (PIECE_IS_SLIDER(piece)) {

				// check for blockers

				inc = DELTA_INC_LINE(delta);
				ASSERT(inc != IncNone);

				S32 sq = *from;
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

bool piece_attack_king(const board_t *board, S32 piece, S32 from, S32 king) {

	ASSERT(board != nullptr);
	ASSERT(piece_is_ok(piece));
	ASSERT(SQUARE_IS_OK(from));
	ASSERT(SQUARE_IS_OK(king));

	const S32 code = PieceCode[piece];
	ASSERT(code >= 0 && code < 4);

	if (PIECE_IS_SLIDER(piece)) {
		S32 delta;
		for (auto delta_ptr = PieceDeltaDelta[code][DeltaOffset + (king - from)].begin();
		     (delta = *delta_ptr) != DeltaNone; ++delta_ptr) {

			ASSERT(delta_is_ok(delta));

			const S32 inc = DeltaIncLine[DeltaOffset + delta];
			ASSERT(inc != IncNone);

			const S32 to = from + delta;

			S32 sq = from;
			do {
				sq += inc;
				if (sq == to && SQUARE_IS_OK(to)) {
					ASSERT(DISTANCE(to, king) == 1);
					return true;
				}
			} while (board->square[sq] == Empty);
		}

	} else { // non-slider

		S32 delta;
		for (auto delta_ptr = PieceDeltaDelta[code][DeltaOffset + (king - from)].begin();
		     (delta = *delta_ptr) != DeltaNone; ++delta_ptr) {

			ASSERT(delta_is_ok(delta));

			const S32 to = from + delta;

			if (SQUARE_IS_OK(to)) {
				ASSERT(DISTANCE(to, king) == 1);
				return true;
			}
		}
	}

	return false;
}

// end of attack.cpp
