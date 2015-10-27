// move.cpp

// includes

#include <cctype>
#include <cstring>
#include <array>
#include "attack.h"
#include "move.h"

// constants

static constexpr std::array<U16, 4> PromotePiece = {Knight64, Bishop64, Rook64, Queen64};

// functions

// move_is_ok()

bool move_is_ok(U16 move) {

	//if (move < 0 || move >= 65536) return false;
	if (move == MoveNone) return false;
	if (move == MoveNull) return false;

	return true;
}

// move_promote()

U16 move_promote(U16 move) {

	ASSERT(move_is_ok(move));
	ASSERT(MOVE_IS_PROMOTE(move));

	S32 code = (move >> 12) & 3;
	S16 piece = PromotePiece[code];

	if (SQUARE_RANK(MOVE_TO(move)) == Rank8)
		piece |= WhiteFlag;
	else {
		ASSERT(SQUARE_RANK(MOVE_TO(move)) == Rank1);
		piece |= BlackFlag;
	}

	ASSERT(piece_is_ok(piece));
	return piece;
}

// move_order()

U16 move_order(U16 move) {

	ASSERT(move_is_ok(move));

	return ((move & 07777) << 2) | ((move >> 12) & 3);
}

// move_is_capture()

bool move_is_capture(U16 move, const board_t *board) {

	ASSERT(move_is_ok(move));
	ASSERT(board != nullptr);

	return MOVE_IS_EN_PASSANT(move) || board->square[MOVE_TO(move)] != Empty;
}

// move_is_under_promote()

bool move_is_under_promote(U16 move) {

	ASSERT(move_is_ok(move));

	return MOVE_IS_PROMOTE(move) && (move & MoveAllFlags) != MovePromoteQueen;
}

// move_is_tactical()

bool move_is_tactical(U16 move, const board_t *board) {

	ASSERT(move_is_ok(move));
	ASSERT(board != nullptr);

	return (move & (1 << 15)) != 0 || board->square[MOVE_TO(move)] != Empty; // HACK
}

// move_capture()

U16 move_capture(U16 move, const board_t *board) {

	ASSERT(move_is_ok(move));
	ASSERT(board != nullptr);

	if (MOVE_IS_EN_PASSANT(move))
		return PAWN_OPP(board->square[MOVE_FROM(move)]);

	return board->square[MOVE_TO(move)];
}

// move_to_string()

bool move_to_string(U16 move, char string[], S32 size) {

	ASSERT(move == MoveNull || move_is_ok(move));
	ASSERT(string != nullptr);
	ASSERT(size >= 6);

	if (size < 6) return false;

	// null move

	if (move == MoveNull) {
		strcpy(string, NullMoveString);
		return true;
	}

	// normal moves

	square_to_string(MOVE_FROM(move), &string[0], 3);
	square_to_string(MOVE_TO(move), &string[2], 3);
	ASSERT(strlen(string) == 4);

	// promotes

	if (MOVE_IS_PROMOTE(move)) {
		string[4] = tolower(piece_to_char(move_promote(move)));
		string[5] = '\0';
	}

	return true;
}

// move_from_string()

U16 move_from_string(const char string[], const board_t *board) {

	ASSERT(string != nullptr);
	ASSERT(board != nullptr);

	// from
	char tmp_string[3];
	tmp_string[0] = string[0];
	tmp_string[1] = string[1];
	tmp_string[2] = '\0';

	const S32 from = square_from_string(tmp_string);
	if (from == SquareNone) return MoveNone;

	// to
	tmp_string[0] = string[2];
	tmp_string[1] = string[3];
	tmp_string[2] = '\0';

	const S32 to = square_from_string(tmp_string);
	if (to == SquareNone) return MoveNone;

	U16 move = MOVE_MAKE(from, to);

	// promote
	switch (string[4]) {
		case '\0': // not a promotion
			break;
		case 'n': move |= MovePromoteKnight;
	        break;
		case 'b': move |= MovePromoteBishop;
	        break;
		case 'r': move |= MovePromoteRook;
	        break;
		case 'q': move |= MovePromoteQueen;
	        break;
		default: return MoveNone;
	}

	// flags

	const S32 piece = board->square[from];

	if (PIECE_IS_PAWN(piece)) {
		if (to == board->ep_square)
			move |= MoveEnPassant;
	} else if (PIECE_IS_KING(piece)) {
		const S32 delta = to - from;
		if (delta == +2 || delta == -2)
			move |= MoveCastle;
	}
	return move;
}

// end of move.cpp
