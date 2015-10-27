// move_gen.cpp

// includes

#include "attack.h"
#include "list.h"
#include "move.h"
#include "move_evasion.h"
#include "move_gen.h"
#include "move_legal.h"

// prototypes

static void add_moves(list_t *list, const board_t *board);
static void add_captures(list_t *list, const board_t *board);
static void add_quiet_moves(list_t *list, const board_t *board);

static void add_promotes(list_t *list, const board_t *board);
static void add_en_passant_captures(list_t *list, const board_t *board);
static void add_castle_moves(list_t *list, const board_t *board);

// functions

// gen_legal_moves()

void gen_legal_moves(list_t *list, board_t *board) {

	ASSERT(list != nullptr);
	ASSERT(board != nullptr);

	attack_t attack[1];
	attack_set(attack, board);

	if (ATTACK_IN_CHECK(attack))
		gen_legal_evasions(list, board, attack);
	else {
		gen_moves(list, board);
		list->filter(board, &pseudo_is_legal, true);
	}

	// debug
	ASSERT(list->is_ok());
}

// gen_moves()

void gen_moves(list_t *list, const board_t *board) {

	ASSERT(list != nullptr);
	ASSERT(board != nullptr);
	ASSERT(!board_is_check(board));

	LIST_CLEAR(list);

	add_moves(list, board);
	add_en_passant_captures(list, board);
	add_castle_moves(list, board);

	// debug
	ASSERT(list->is_ok());
}

// gen_captures()

void gen_captures(list_t *list, const board_t *board) {

	ASSERT(list != nullptr);
	ASSERT(board != nullptr);

	LIST_CLEAR(list);

	add_captures(list, board);
	add_en_passant_captures(list, board);

	// debug
	ASSERT(list->is_ok());
}

// gen_quiet_moves()

void gen_quiet_moves(list_t *list, const board_t *board) {

	ASSERT(list != nullptr);
	ASSERT(board != nullptr);
	ASSERT(!board_is_check(board));

	LIST_CLEAR(list);

	add_quiet_moves(list, board);
	add_castle_moves(list, board);

	// debug
	ASSERT(list->is_ok());
}

// add_moves()

static void add_moves(list_t *list, const board_t *board) {

	ASSERT(list != nullptr);
	ASSERT(board != nullptr);

	const S8 me = board->turn, opp = COLOUR_OPP(me), opp_flag = COLOUR_FLAG(opp);

	// piece moves
	for (auto from = board->piece[me].begin(); from != board->piece[me].end(); ++from) {

		const S32 piece = board->square[*from];
		const inc_t *inc_ptr = PIECE_INC(piece);

		if (PIECE_IS_SLIDER(piece)) {
			S32 inc;
			for (; (inc = *inc_ptr) != IncNone; ++inc_ptr) {
				S32 capture, to;
				for (to = *from + inc; (capture = board->square[to]) == Empty; to += inc) {
					LIST_ADD(list, MOVE_MAKE(*from, to));
				}
				if (FLAG_IS(capture, opp_flag)) {
					LIST_ADD(list, MOVE_MAKE(*from, to));
				}
			}
		} else {

			S32 inc;
			for (; (inc = *inc_ptr) != IncNone; ++inc_ptr) {
				const S32 to = *from + inc, capture = board->square[to];
				if (capture == Empty || FLAG_IS(capture, opp_flag)) {
					LIST_ADD(list, MOVE_MAKE(*from, to));
				}
			}
		}
	}

	// pawn moves
	const S32 inc = PAWN_MOVE_INC(me);

	for (auto from = board->pawn[me].begin(); from != board->pawn[me].end(); ++from) {

		S32 to = *from + (inc - 1);
		if (FLAG_IS(board->square[to], opp_flag))
			add_pawn_move(list, *from, to);

		to = *from + (inc + 1);
		if (FLAG_IS(board->square[to], opp_flag))
			add_pawn_move(list, *from, to);

		to = *from + inc;
		if (board->square[to] == Empty) {
			add_pawn_move(list, *from, to);
			if (PAWN_RANK(*from, me) == Rank2) {
				to = *from + (2 * inc);
				if (board->square[to] == Empty) {
					ASSERT(!SQUARE_IS_PROMOTE(to));
					LIST_ADD(list, MOVE_MAKE(*from, to));
				}
			}
		}
	}
}

// add_captures()

static void add_captures(list_t *list, const board_t *board) {

	ASSERT(list != nullptr);
	ASSERT(board != nullptr);

	const S8 me = board->turn, opp = COLOUR_OPP(me);
	const S32 opp_flag = COLOUR_FLAG(opp);

	// piece captures
	for (auto ptr = board->piece[me].begin(); ptr != board->piece[me].end(); ++ptr) {
		S32 piece = board->square[*ptr];

		switch (PIECE_TYPE(piece)) {
			case Knight64: {
				for (U8 i = 0; i < knight_moves.size(); ++i) {
					S32 to = *ptr + knight_moves[i];
					if (FLAG_IS(board->square[to], opp_flag)) LIST_ADD(list, MOVE_MAKE(*ptr, to));
				}

				break;
			}

			case Bishop64: {
				for (U8 i = 0; i < bishop_moves.size(); ++i) {
					S32 to, capture;
					for (to = *ptr + bishop_moves[i]; (capture = board->square[to]) == Empty; to += bishop_moves[i]);
					if (FLAG_IS(capture, opp_flag)) LIST_ADD(list, MOVE_MAKE(*ptr, to));
				}
				break;
			}

			case Rook64: {
				for (U8 i = 0; i < rook_moves.size(); ++i) {
					S32 to, capture;
					for (to = *ptr + rook_moves[i]; (capture = board->square[to]) == Empty; to += rook_moves[i]);
					if (FLAG_IS(capture, opp_flag)) LIST_ADD(list, MOVE_MAKE(*ptr, to));
				}
				break;
			}

			case Queen64: {
				for (U8 i = 0; i < queen_moves.size(); ++i) {
					S32 to, capture;
					for (to = *ptr + queen_moves[i]; (capture = board->square[to]) == Empty; to += queen_moves[i]);
					if (FLAG_IS(capture, opp_flag)) LIST_ADD(list, MOVE_MAKE(*ptr, to));
				}
				break;
			}

			case King64: {
				for (U8 i = 0; i < king_moves.size(); ++i) {
					S32 to = *ptr + king_moves[i];
					if (FLAG_IS(board->square[to], opp_flag)) LIST_ADD(list, MOVE_MAKE(*ptr, to));
				}

				break;
			}

			default: ASSERT(false);
		        break;
		}
	}

	// pawn captures

	if (COLOUR_IS_WHITE(me)) {

		for (auto ptr = board->pawn[me].begin(); ptr != board->pawn[me].end(); ++ptr) {
			S32 to;
			to = *ptr + 15;
			if (FLAG_IS(board->square[to], opp_flag)) add_pawn_move(list, *ptr, to);

			to = *ptr + 17;
			if (FLAG_IS(board->square[to], opp_flag)) add_pawn_move(list, *ptr, to);

			// promote
			if (SQUARE_RANK(*ptr) == Rank7) {
				to = *ptr + 16;
				if (board->square[to] == Empty)
					add_promote(list, MOVE_MAKE(*ptr, to));
			}
		}
	} else { // black


		for (auto ptr = board->pawn[me].begin(); ptr != board->pawn[me].end(); ++ptr) {
			S32 to;
			to = *ptr - 17;
			if (FLAG_IS(board->square[to], opp_flag)) add_pawn_move(list, *ptr, to);

			to = *ptr - 15;
			if (FLAG_IS(board->square[to], opp_flag)) add_pawn_move(list, *ptr, to);

			// promote

			if (SQUARE_RANK(*ptr) == Rank2) {
				to = *ptr - 16;
				if (board->square[to] == Empty)
					add_promote(list, MOVE_MAKE(*ptr, to));
			}
		}
	}
}

// add_quiet_moves()

static void add_quiet_moves(list_t *list, const board_t *board) {

	ASSERT(list != nullptr);
	ASSERT(board != nullptr);

	const S8 me = board->turn;

	// piece moves
	for (auto from = board->piece[me].begin(); from != board->piece[me].end(); ++from) {
		const S32 piece = board->square[*from];

		switch (PIECE_TYPE(piece)) {

			case Knight64:

				for (U8 i = 0; i < knight_moves.size(); ++i) {
					S32 to = *from + knight_moves[i];
					if (board->square[to] == Empty) LIST_ADD(list, MOVE_MAKE(*from, to));
				}
		        break;

			case Bishop64:

				for (U8 i = 0; i < bishop_moves.size(); ++i) {
					for (S32 to = *from + bishop_moves[i]; board->square[to] == Empty; to += bishop_moves[i])
						LIST_ADD(list, MOVE_MAKE(*from, to));
				}
		        break;

			case Rook64:

				for (U8 i = 0; i < rook_moves.size(); ++i) {
					for (S32 to = *from + rook_moves[i]; board->square[to] == Empty; to += rook_moves[i])
						LIST_ADD(list, MOVE_MAKE(*from, to));
				}
		        break;

			case Queen64:

				for (U8 i = 0; i < queen_moves.size(); ++i) {
					for (S32 to = *from + queen_moves[i]; board->square[to] == Empty; to += queen_moves[i])
						LIST_ADD(list, MOVE_MAKE(*from, to));
				}
		        break;

			case King64:

				for (U8 i = 0; i < king_moves.size(); ++i) {
					S32 to = *from + king_moves[i];
					if (board->square[to] == Empty) LIST_ADD(list, MOVE_MAKE(*from, to));
				}
		        break;

			default: ASSERT(false);
		        break;
		}
	}

	// pawn moves
	if (COLOUR_IS_WHITE(me)) {

		for (auto from = board->pawn[me].begin(); from != board->pawn[me].end(); ++from) {

			// non promotes
			if (SQUARE_RANK(*from) != Rank7) {
				S32 to = *from + 16;
				if (board->square[to] == Empty) {
					ASSERT(!SQUARE_IS_PROMOTE(to));
					LIST_ADD(list, MOVE_MAKE(*from, to));
					if (SQUARE_RANK(*from) == Rank2) {
						to = *from + 32;
						if (board->square[to] == Empty) {
							ASSERT(!SQUARE_IS_PROMOTE(to));
							LIST_ADD(list, MOVE_MAKE(*from, to));
						}
					}
				}
			}
		}
	} else { // black

		for (auto from = board->pawn[me].begin(); from != board->pawn[me].end(); ++from) {

			// non promotes
			if (SQUARE_RANK(*from) != Rank2) {
				S32 to = *from - 16;
				if (board->square[to] == Empty) {
					ASSERT(!SQUARE_IS_PROMOTE(to));
					LIST_ADD(list, MOVE_MAKE(*from, to));
					if (SQUARE_RANK(*from) == Rank7) {
						to = *from - 32;
						if (board->square[to] == Empty) {
							ASSERT(!SQUARE_IS_PROMOTE(to));
							LIST_ADD(list, MOVE_MAKE(*from, to));
						}
					}
				}
			}
		}
	}
}

// add_promotes()

static void add_promotes(list_t *list, const board_t *board) {

	ASSERT(list != nullptr);
	ASSERT(board != nullptr);

	const S8 me = board->turn;
	const S32 inc = PAWN_MOVE_INC(me);

	for (auto from = board->pawn[me].begin(); from != board->pawn[me].end(); ++from)
		if (PAWN_RANK(*from, me) == Rank7) {
			const S32 to = *from + inc;
			if (board->square[to] == Empty)
				add_promote(list, MOVE_MAKE(*from, to));
		}
}

// add_en_passant_captures()

static void add_en_passant_captures(list_t *list, const board_t *board) {

	ASSERT(list != nullptr);
	ASSERT(board != nullptr);

	const S32 to = board->ep_square;

	if (to != SquareNone) {

		const S8 me = board->turn;
		const S32 inc = PAWN_MOVE_INC(me), pawn = PAWN_MAKE(me);
		S32 from = to - (inc - 1);

		if (board->square[from] == pawn) {
			ASSERT(!SQUARE_IS_PROMOTE(to));
			LIST_ADD(list, MOVE_MAKE_FLAGS(from, to, MoveEnPassant));
		}

		from = to - (inc + 1);
		if (board->square[from] == pawn) {
			ASSERT(!SQUARE_IS_PROMOTE(to));
			LIST_ADD(list, MOVE_MAKE_FLAGS(from, to, MoveEnPassant));
		}
	}
}

// add_castle_moves()

static void add_castle_moves(list_t *list, const board_t *board) {

	ASSERT(list != nullptr);
	ASSERT(board != nullptr);
	ASSERT(!board_is_check(board));

	if (COLOUR_IS_WHITE(board->turn)) {

		if ((board->flags & FlagsWhiteKingCastle) != 0 && board->square[F1] == Empty && board->square[G1] == Empty
			&& !is_attacked(board, F1, Black))
			LIST_ADD(list, MOVE_MAKE_FLAGS(E1, G1, MoveCastle));

		if ((board->flags & FlagsWhiteQueenCastle) != 0 && board->square[D1] == Empty && board->square[C1] == Empty
			&& board->square[B1] == Empty && !is_attacked(board, D1, Black))
			LIST_ADD(list, MOVE_MAKE_FLAGS(E1, C1, MoveCastle));

	} else { // black

		if ((board->flags & FlagsBlackKingCastle) != 0 && board->square[F8] == Empty && board->square[G8] == Empty
			&& !is_attacked(board, F8, White))
			LIST_ADD(list, MOVE_MAKE_FLAGS(E8, G8, MoveCastle));

		if ((board->flags & FlagsBlackQueenCastle) != 0 && board->square[D8] == Empty && board->square[C8] == Empty
			&& board->square[B8] == Empty && !is_attacked(board, D8, White))
			LIST_ADD(list, MOVE_MAKE_FLAGS(E8, C8, MoveCastle));
	}
}

// add_pawn_move()

void add_pawn_move(list_t *list, S32 from, S32 to) {

	ASSERT(list != nullptr);
	ASSERT(SQUARE_IS_OK(from));
	ASSERT(SQUARE_IS_OK(to));

	U16 move = MOVE_MAKE(from, to);

	if (SQUARE_IS_PROMOTE(to)) {
		LIST_ADD(list, move | MovePromoteQueen);//add_promote
		LIST_ADD(list, move | MovePromoteKnight);
		LIST_ADD(list, move | MovePromoteRook);
		LIST_ADD(list, move | MovePromoteBishop);
	} else
		LIST_ADD(list, move);
}

// add_promote()

void add_promote(list_t *list, U16 move) {

	ASSERT(list != nullptr);
	ASSERT(move_is_ok(move));
	ASSERT((move & ~07777) == 0); // HACK
	ASSERT(SQUARE_IS_PROMOTE(MOVE_TO(move)));

	LIST_ADD(list, move | MovePromoteQueen);
	LIST_ADD(list, move | MovePromoteKnight);
	LIST_ADD(list, move | MovePromoteRook);
	LIST_ADD(list, move | MovePromoteBishop);
}

// end of move_gen.cpp
