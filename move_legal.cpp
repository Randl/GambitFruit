// move_legal.cpp

// includes

#include "attack.h"
#include "list.h"
#include "move.h"
#include "move_do.h"
#include "move_gen.h"
#include "move_legal.h"

// prototypes

static bool move_is_pseudo_debug(U16 move, board_t *board);

// functions

// move_is_pseudo()

bool move_is_pseudo(U16 move, board_t *board) {

	ASSERT(move_is_ok(move));
	ASSERT(board != nullptr);

	ASSERT(!board_is_check(board));

	// special cases
	if (MOVE_IS_SPECIAL(move))
		return move_is_pseudo_debug(move, board);

	ASSERT((move & ~07777) == 0);

	// init
	const S8 me = board->turn;//, opp = COLOUR_OPP(board->turn);

	// from
	const S32 from = MOVE_FROM(move);
	ASSERT(SQUARE_IS_OK(from));

	const S32 piece = board->square[from];
	if (!COLOUR_IS(piece, me)) return false;
	ASSERT(piece_is_ok(piece));

	// to
	const S32 to = MOVE_TO(move);
	ASSERT(SQUARE_IS_OK(to));

	const S32 capture = board->square[to];
	if (COLOUR_IS(capture, me)) return false;

	// move
	if (PIECE_IS_PAWN(piece)) {
		if (SQUARE_IS_PROMOTE(to)) return false;

		const S32 inc = PAWN_MOVE_INC(me), delta = to - from;
		ASSERT(delta_is_ok(delta));

		if (capture == Empty) {

			// pawn push
			if (delta == inc) return true;

			if (delta == (2 * inc)
				&& PAWN_RANK(from, me) == Rank2
				&& board->square[from + inc] == Empty)
				return true;

		} else {
			// pawn capture
			if (delta == (inc - 1) || delta == (inc + 1)) return true;
		}

	} else {
		if (PIECE_ATTACK(board, piece, from, to)) return true;
	}

	return false;
}

// quiet_is_pseudo()

bool quiet_is_pseudo(U16 move, board_t *board) {

	ASSERT(move_is_ok(move));
	ASSERT(board != nullptr);
	ASSERT(!board_is_check(board));

	// special cases
	if (MOVE_IS_CASTLE(move))
		return move_is_pseudo_debug(move, board);
	else if (MOVE_IS_SPECIAL(move))
		return false;


	ASSERT((move & ~07777) == 0);

	// init
	const S8 me = board->turn;// opp = COLOUR_OPP(board->turn);

	// from
	const S32 from = MOVE_FROM(move);
	ASSERT(SQUARE_IS_OK(from));

	const S32 piece = board->square[from];
	if (!COLOUR_IS(piece, me)) return false;

	ASSERT(piece_is_ok(piece));

	// to
	const S32 to = MOVE_TO(move);
	ASSERT(SQUARE_IS_OK(to));

	if (board->square[to] != Empty) return false; // capture

	// move
	if (PIECE_IS_PAWN(piece)) {
		if (SQUARE_IS_PROMOTE(to)) return false;
		const S32 inc = PAWN_MOVE_INC(me), delta = to - from;
		ASSERT(delta_is_ok(delta));

		// pawn push
		if (delta == inc) return true;

		if (delta == (2 * inc)
			&& PAWN_RANK(from, me) == Rank2
			&& board->square[from + inc] == Empty)
			return true;
	} else if (PIECE_ATTACK(board, piece, from, to))
		return true;

	return false;
}

// pseudo_is_legal()

bool pseudo_is_legal(U16 move, board_t *board) {

	ASSERT(move_is_ok(move));
	ASSERT(board != nullptr);

	// init
	const S8 me = board->turn, opp = COLOUR_OPP(me);
	const S32 from = MOVE_FROM(move), to = MOVE_TO(move), piece = board->square[from];
	ASSERT(COLOUR_IS(piece, me));

	// slow test for en-passant captures
	if (MOVE_IS_EN_PASSANT(move)) {
		undo_t undo[1];
		move_do(board, move, undo);
		bool legal = !IS_IN_CHECK(board, me);
		move_undo(board, move, undo);
		return legal;
	}

	// king moves (including castle)
	if (PIECE_IS_KING(piece)) {

		bool legal = !is_attacked(board, to, opp);

		if (DEBUG) {
			ASSERT(board->square[from] == piece);
			board->square[from] = Empty;
			ASSERT(legal == !is_attacked(board, to, opp));
			board->square[from] = piece;
		}

		return legal;
	}

	// pins

	if (is_pinned(board, from, me)) {
		const S32 king = KING_POS(board, me);
		return DELTA_INC_LINE(king - to) == DELTA_INC_LINE(king - from); // does not discover the line
	}

	return true;
}

// move_is_pseudo_debug()

static bool move_is_pseudo_debug(U16 move, board_t *board) {

	ASSERT(move_is_ok(move));
	ASSERT(board != nullptr);
	ASSERT(!board_is_check(board));

	list_t list[1];
	gen_moves(list, board);
	return list->contains(move);
}

// end of move_legal.cpp
