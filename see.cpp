
// see.cpp

// includes

#include "attack.h"
#include "board.h"
#include "colour.h"
#include "move.h"
#include "piece.h"
#include "see.h"
#include "util.h"
#include "value.h"

// macros

#define ALIST_CLEAR(alist) ((alist)->size=0)

// types

struct alist_t {
	int_fast32_t square[15];
	int_fast8_t size;
};

struct alists_t {
	alist_t alist[ColourNb][1];
};

// prototypes

static int_fast32_t  see_rec       (alists_t * alists, const board_t * board, int_fast8_t colour, int_fast32_t to, int_fast32_t piece_value);

static void alist_build   (alist_t * alist, const board_t * board, int_fast32_t to, int_fast8_t colour);
static void alists_hidden (alists_t * alists, const board_t * board, int_fast32_t from, int_fast32_t to);

static void alist_clear   (alist_t * alist);
static void alist_add     (alist_t * alist, int_fast32_t square, const board_t * board);
static void alist_remove  (alist_t * alist, int_fast32_t pos);
static int_fast32_t  alist_pop     (alist_t * alist);

// functions

// see_move()

int_fast32_t see_move(int_fast32_t move, const board_t * board) {

	ASSERT(move_is_ok(move));
	ASSERT(board!=nullptr);

	// init
	const int_fast32_t from = MOVE_FROM(move), to = MOVE_TO(move);

	// move the piece

	int_fast32_t piece_value = 0, piece = board->square[from];
	ASSERT(piece_is_ok(piece));

	int_fast32_t att = PIECE_COLOUR(piece), def = COLOUR_OPP(att);

	// promote
	if (MOVE_IS_PROMOTE(move)) {
		ASSERT(PIECE_IS_PAWN(piece));
		piece = move_promote(move);
		ASSERT(piece_is_ok(piece));
		ASSERT(COLOUR_IS(piece,att));
	}

	piece_value += VALUE_PIECE(piece);

	// clear attacker lists
	alists_t alists[1];
	ALIST_CLEAR(alists->alist[Black]);
	ALIST_CLEAR(alists->alist[White]);

	// find hidden attackers
	alists_hidden(alists,board,from,to);

	// capture the piece
	int_fast32_t value = 0, capture = board->square[to];

	if (capture != Empty) {
		ASSERT(piece_is_ok(capture));
		ASSERT(COLOUR_IS(capture,def));

		value += VALUE_PIECE(capture);
	}

	// promote
	if (MOVE_IS_PROMOTE(move))
		value += VALUE_PIECE(piece) - ValuePawn;

	// en-passant
	if (MOVE_IS_EN_PASSANT(move)) {
		ASSERT(value==0);
		ASSERT(PIECE_IS_PAWN(board->square[SQUARE_EP_DUAL(to)]));
		value += ValuePawn;
		alists_hidden(alists,board,SQUARE_EP_DUAL(to),to);
	}

	// build defender list
	alist_t *alist = alists->alist[def];
	alist_build(alist,board,to,def);
	if (alist->size == 0) return value; // no defender => stop SEE

	// build attacker list
	alist = alists->alist[att];
	alist_build(alist,board,to,att);

	// remove the moved piece (if it's an attacker)
	int_fast32_t pos;
	for (pos = 0; pos < alist->size && alist->square[pos] != from; ++pos);
	if (pos < alist->size) alist_remove(alist,pos);

	// SEE search
	value -= see_rec(alists,board,def,to,piece_value);
	return value;
}

// see_square()
/*
int_fast32_t see_square(const board_t * board, int_fast32_t to, int_fast8_t colour) {

   int_fast32_t att, def;
   alists_t alists[1];
   alist_t * alist;
   int_fast32_t piece_value;
   int_fast32_t piece;

   ASSERT(board!=nullptr);
   ASSERT(SQUARE_IS_OK(to));
   ASSERT(COLOUR_IS_OK(colour));

   ASSERT(COLOUR_IS(board->square[to],COLOUR_OPP(colour)));

   // build attacker list

   att = colour;
   alist = alists->alist[att];

   ALIST_CLEAR(alist);
   alist_build(alist,board,to,att);

   if (alist->size == 0) return 0; // no attacker => stop SEE

   // build defender list

   def = COLOUR_OPP(att);
   alist = alists->alist[def];

   ALIST_CLEAR(alist);
   alist_build(alist,board,to,def);

   // captured piece

   piece = board->square[to];
   ASSERT(piece_is_ok(piece));
   ASSERT(COLOUR_IS(piece,def));

   piece_value = VALUE_PIECE(piece);

   // SEE search

   return see_rec(alists,board,att,to,piece_value);
}
*/
// see_rec()

static int_fast32_t see_rec(alists_t * alists, const board_t * board, int_fast8_t colour, int_fast32_t to, int_fast32_t piece_value) {

	ASSERT(alists!=nullptr);
	ASSERT(board!=nullptr);
	ASSERT(COLOUR_IS_OK(colour));
	ASSERT(SQUARE_IS_OK(to));
	ASSERT(piece_value>0);

	// find the least valuable attacker
	int_fast32_t from = alist_pop(alists->alist[colour]);
	if (from == SquareNone) return 0; // no more attackers

	// find hidden attackers
	alists_hidden(alists,board,from,to);

	// calculate the capture value
	int_fast32_t value = +piece_value; // captured piece
	if (value == ValueKing) return value; // do not allow an answer to a king capture

	int_fast32_t piece = board->square[from];
	ASSERT(piece_is_ok(piece));
	ASSERT(COLOUR_IS(piece,colour));
	piece_value = VALUE_PIECE(piece);

	// promote
	if (piece_value == ValuePawn && SQUARE_IS_PROMOTE(to)) { // HACK: PIECE_IS_PAWN(piece)
		ASSERT(PIECE_IS_PAWN(piece));
		piece_value = ValueQueen;
		value += ValueQueen - ValuePawn;
   }

	value -= see_rec(alists,board,COLOUR_OPP(colour),to,piece_value);

	if (value < 0) value = 0;

	return value;
}

// alist_build()

static void alist_build(alist_t * alist, const board_t * board, int_fast32_t to, int_fast8_t colour) {

	ASSERT(alist!=nullptr);
	ASSERT(board!=nullptr);
	ASSERT(SQUARE_IS_OK(to));
	ASSERT(COLOUR_IS_OK(colour));

	// piece attacks
	for (const sq_t *ptr = &board->piece[colour][0]; *ptr != SquareNone; ++ptr) {
		const int_fast32_t piece = board->square[*ptr], delta = to - *ptr;

		if (PSEUDO_ATTACK(piece,delta)) {
			int_fast32_t inc = DELTA_INC_ALL(delta);
			ASSERT(inc!=IncNone);

			int_fast32_t sq = *ptr;
			do {
				sq += inc;
				if (sq == to) { // attack
					alist_add(alist,*ptr,board);
					break;
				}
			} while (board->square[sq] == Empty);
		}
	}

	// pawn attacks
	int_fast32_t inc = PAWN_MOVE_INC(colour), pawn = PAWN_MAKE(colour), from = to - (inc-1);

	if (board->square[from] == pawn) alist_add(alist,from,board);
	from = to - (inc+1);
	if (board->square[from] == pawn) alist_add(alist,from,board);
}

// alists_hidden()

static void alists_hidden(alists_t * alists, const board_t * board, int_fast32_t from, int_fast32_t to) {

	ASSERT(alists!=nullptr);
	ASSERT(board!=nullptr);
	ASSERT(SQUARE_IS_OK(from));
	ASSERT(SQUARE_IS_OK(to));

	int_fast32_t inc = DELTA_INC_LINE(to-from);

	if (inc != IncNone) { // line

		int_fast32_t sq = from, piece;
		do sq -= inc; while ((piece=board->square[sq]) == Empty);

		if (SLIDER_ATTACK(piece,inc)) {

			ASSERT(piece_is_ok(piece));
			ASSERT(PIECE_IS_SLIDER(piece));

			alist_add(alists->alist[PIECE_COLOUR(piece)],sq,board);
		}
	}
}

// alist_clear()

static void alist_clear(alist_t * alist) {
	ASSERT(alist!=nullptr);
	alist->size = 0;
}

// alist_add()

static void alist_add(alist_t * alist, int_fast32_t square, const board_t * board) {

	ASSERT(alist!=nullptr);
	ASSERT(SQUARE_IS_OK(square));
	ASSERT(board!=nullptr);

	// insert in MV order
	int_fast32_t piece = board->square[square], size = ++alist->size; // HACK

	ASSERT(size>0&&size<16);

	int_fast32_t pos;
	for (pos = size-1; pos > 0 && piece > board->square[alist->square[pos-1]]; --pos) { // HACK
		ASSERT(pos>0&&pos<size);
		alist->square[pos] = alist->square[pos-1];
	}

	ASSERT(pos>=0&&pos<size);
	alist->square[pos] = square;
}

// alist_remove()

static void alist_remove(alist_t * alist, int_fast32_t pos) {

	ASSERT(alist!=nullptr);
	ASSERT(pos>=0&&pos<alist->size);

	int_fast32_t size = alist->size--; // HACK
	ASSERT(size>=1);

	ASSERT(pos>=0&&pos<size);

	for (int_fast32_t i = pos; i < size-1; ++i) {
		ASSERT(i>=0&&i<size-1);
		alist->square[i] = alist->square[i+1];
	}
}

// alist_pop()

static int_fast32_t alist_pop(alist_t * alist) {

	ASSERT(alist!=nullptr);
	ASSERT(board!=nullptr);

	int_fast32_t sq = SquareNone, size = alist->size;

	if (size != 0) {
		--size;
		ASSERT(size>=0);
		sq = alist->square[size];
		alist->size = size;
	}

	return sq;
}

// end of see.cpp
