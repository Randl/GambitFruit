//
// Created by Evgenii on 26.08.2015.
//

#include "value.h"
#include "bitbases.h"

#define ADD_PIECE(type)  {\
         egbb_piece[total_pieces] = type;\
         egbb_square[total_pieces] = from;\
        ++total_pieces;\
};
constexpr int_fast8_t max_pieces = 32;

bool bitbase_probe(const board_t *board, int_fast32_t value) {

	const int_fast8_t player       = board->turn;
	int_fast8_t       total_pieces = 2;
	int_fast32_t      egbb_piece[max_pieces], egbb_square[max_pieces];

	for (int_fast32_t i = 0; i < max_pieces; ++i) {
		egbb_piece[i]  = 0;
		egbb_square[i] = 0;
	}

	egbb_piece[0] = _WKING;
	egbb_piece[1] = _BKING;
	for (int_fast8_t from = 0; from < 64; ++from) {
		if (board->square[SQUARE_FROM_64(from)] == Empty) continue;

		switch (board->square[SQUARE_FROM_64(from)]) {
			case WP: ADD_PIECE(_WPAWN);
		        break;
			case BP: ADD_PIECE(_BPAWN);
		        break;
			case WN: ADD_PIECE(_WKNIGHT);
		        break;
			case BN: ADD_PIECE(_BKNIGHT);
		        break;
			case WB: ADD_PIECE(_WBISHOP);
		        break;
			case BB: ADD_PIECE(_BBISHOP);
		        break;
			case WR: ADD_PIECE(_WROOK);
		        break;
			case BR: ADD_PIECE(_BROOK);
		        break;
			case WQ: ADD_PIECE(_WQUEEN);
		        break;
			case BQ: ADD_PIECE(_BQUEEN);
		        break;
			case WK:
				egbb_square[0] = from;
		        break;
			case BK:
				egbb_square[1] = from;
		        break;
			default:
				break;
		}
	}

	const int_fast32_t score = probe_egbb(player, egbb_piece, egbb_square);

	if (score != _NOTFOUND) {
		if (score == 0)
			value = ValueDraw;
		else
			value = score;
		return true;
	}
	return false;
}