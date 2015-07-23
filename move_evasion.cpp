
// move_evasion.cpp

// includes

#include "attack.h"
#include "board.h"
#include "colour.h"
#include "list.h"
#include "move.h"
#include "move_evasion.h"
#include "move_gen.h"
#include "piece.h"
#include "util.h"

// prototypes

static bool gen_evasions      (list_t * list, const board_t * board, const attack_t * attack, bool legal, bool stop);

static bool add_pawn_moves    (list_t * list, const board_t * board, int_fast32_t to, bool legal, bool stop);
static bool add_pawn_captures (list_t * list, const board_t * board, int_fast32_t to, bool legal, bool stop);
static bool add_piece_moves   (list_t * list, const board_t * board, int_fast32_t to, bool legal, bool stop);

// functions

// gen_legal_evasions()

void gen_legal_evasions(list_t * list, const board_t * board, const attack_t * attack) {

	ASSERT(list!=nullptr);
	ASSERT(board!=nullptr);
	ASSERT(attack!=nullptr);

	gen_evasions(list,board,attack,true,false);

	// debug
	ASSERT(list_is_ok(list));
}

// gen_pseudo_evasions()

void gen_pseudo_evasions(list_t * list, const board_t * board, const attack_t * attack) {

	ASSERT(list!=nullptr);
	ASSERT(board!=nullptr);
	ASSERT(attack!=nullptr);

	gen_evasions(list,board,attack,false,false);

	// debug
	ASSERT(list_is_ok(list));
}

// legal_evasion_exist()

bool legal_evasion_exist(const board_t * board, const attack_t * attack) {

	list_t *list; // dummy

	ASSERT(board!=nullptr);
	ASSERT(attack!=nullptr);

	return gen_evasions(list,board,attack,true,true);
}

// gen_evasions()

static bool gen_evasions(list_t * list, const board_t * board, const attack_t * attack, bool legal, bool stop) {

	ASSERT(list!=nullptr);
	ASSERT(board!=nullptr);
	ASSERT(attack!=nullptr);
	ASSERT(legal==true||legal==false);
	ASSERT(stop==true||stop==false);

	ASSERT(board_is_check(board));
	ASSERT(ATTACK_IN_CHECK(attack));

	// init
	LIST_CLEAR(list);
	const int_fast32_t me = board->turn, opp = COLOUR_OPP(me);
	const int_fast32_t opp_flag = COLOUR_FLAG(opp), king = KING_POS(board,me);

	int_fast32_t inc;
	for (auto inc_ptr = KingInc.begin(); (inc=*inc_ptr) != IncNone; ++inc_ptr) {
		if (inc != -attack->di[0] && inc != -attack->di[1]) { // avoid escaping along a check line
			const int_fast32_t to = king + inc, piece = board->square[to];
			if (piece == Empty || FLAG_IS(piece,opp_flag)) {
				if (!legal || !is_attacked(board,to,opp)) {
					if (stop) return true;
					LIST_ADD(list,MOVE_MAKE(king,to));
				}
			}
		}
	}

	if (attack->dn >= 2) return false; // double check, we are done

	// single check
	ASSERT(attack->dn==1);

	// capture the checking piece
	if (add_pawn_captures(list,board,attack->ds[0],legal,stop) && stop) return true;
	if (add_piece_moves(list,board,attack->ds[0],legal,stop) && stop) return true;

	// interpose a piece
	const int_fast32_t inc2 = attack->di[0];
	if (inc2 != IncNone) { // line
		for (int_fast32_t to = king+inc2; to != attack->ds[0]; to += inc2) {
			ASSERT(SQUARE_IS_OK(to));
			ASSERT(board->square[to]==Empty);
			if (add_pawn_moves(list,board,to,legal,stop) && stop) return true;
			if (add_piece_moves(list,board,to,legal,stop) && stop) return true;
		}
	}

	return false;
}

// add_pawn_moves()

static bool add_pawn_moves(list_t * list, const board_t * board, int_fast32_t to, bool legal, bool stop) {

	ASSERT(list!=nullptr);
	ASSERT(board!=nullptr);
	ASSERT(SQUARE_IS_OK(to));
	ASSERT(legal==true||legal==false);
	ASSERT(stop==true||stop==false);
	ASSERT(board->square[to]==Empty);

	const int_fast32_t me = board->turn, inc = PAWN_MOVE_INC(me), pawn = PAWN_MAKE(me);
	int_fast32_t from = to - inc;
	const int_fast32_t piece = board->square[from];

	if (piece == pawn) { // single push
		if (!legal || !is_pinned(board,from,me)) {
			if (stop) return true;
			add_pawn_move(list,from,to);
		}
	} else if (piece == Empty && PAWN_RANK(to,me) == Rank4) { // double push

		from = to - (2*inc);
		if (board->square[from] == pawn) {
			if (!legal || !is_pinned(board,from,me)) {
				if (stop) return true;
				ASSERT(!SQUARE_IS_PROMOTE(to));
				LIST_ADD(list,MOVE_MAKE(from,to));
			}
		}
	}

	return false;
}

// add_pawn_captures()

static bool add_pawn_captures(list_t * list, const board_t * board, int_fast32_t to, bool legal, bool stop) {

	ASSERT(list!=nullptr);
	ASSERT(board!=nullptr);
	ASSERT(SQUARE_IS_OK(to));
	ASSERT(legal==true||legal==false);
	ASSERT(stop==true||stop==false);
	ASSERT(COLOUR_IS(board->square[to],COLOUR_OPP(board->turn)));

	const int_fast32_t me = board->turn, inc = PAWN_MOVE_INC(me), pawn = PAWN_MAKE(me);

	int_fast32_t from = to - (inc-1);
	if (board->square[from] == pawn) {
		if (!legal || !is_pinned(board,from,me)) {
			if (stop) return true;
			add_pawn_move(list,from,to);
		}
	}

	from = to - (inc+1);
	if (board->square[from] == pawn) {
		if (!legal || !is_pinned(board,from,me)) {
			if (stop) return true;
			add_pawn_move(list,from,to);
		}
	}

	if (board->ep_square != SquareNone
	 && to == SQUARE_EP_DUAL(board->ep_square)) {

		ASSERT(PAWN_RANK(to,me)==Rank5);
		ASSERT(PIECE_IS_PAWN(board->square[to]));

		to = board->ep_square;
		ASSERT(PAWN_RANK(to,me)==Rank6);
		ASSERT(board->square[to]==Empty);

		from = to - (inc-1);
		if (board->square[from] == pawn) {
			if (!legal || !is_pinned(board,from,me)) {
				if (stop) return true;
				ASSERT(!SQUARE_IS_PROMOTE(to));
				LIST_ADD(list,MOVE_MAKE_FLAGS(from,to,MoveEnPassant));
			}
		}

		from = to - (inc+1);
		if (board->square[from] == pawn) {
			if (!legal || !is_pinned(board,from,me)) {
				if (stop) return true;
					ASSERT(!SQUARE_IS_PROMOTE(to));
					LIST_ADD(list,MOVE_MAKE_FLAGS(from,to,MoveEnPassant));
			}
		}
	}

	return false;
}

// add_piece_moves()

static bool add_piece_moves(list_t * list, const board_t * board, int_fast32_t to, bool legal, bool stop) {

	ASSERT(list!=nullptr);
	ASSERT(board!=nullptr);
	ASSERT(SQUARE_IS_OK(to));
	ASSERT(legal==true||legal==false);
	ASSERT(stop==true||stop==false);

	const int_fast32_t me = board->turn;
	int_fast32_t from;
	for (const sq_t *ptr = &board->piece[me][1]; (from=*ptr) != SquareNone; ++ptr) { // HACK: no king
		const int_fast32_t piece = board->square[from];
		if (PIECE_ATTACK(board,piece,from,to)) {
			if (!legal || !is_pinned(board,from,me)) {
				if (stop) return true;
				LIST_ADD(list,MOVE_MAKE(from,to));
			}
		}
	}

	return false;
}

// end of move_evasion.cpp