
// hash.cpp

// includes

#include "board.h"
#include "hash.h"
#include "piece.h"
#include "random.h"
#include "square.h"
#include "util.h"


// variables

std::array<uint_fast64_t, 16> Castle64;

// prototypes

static uint_fast64_t hash_counter_key (int_fast32_t piece_12, int_fast32_t count);

// functions

// hash_init()

void hash_init() {
	for (int_fast32_t i = 0; i < 16; ++i)
		Castle64[i] = hash_castle_key(i);
}

// hash_key()

uint_fast64_t hash_key(const board_t * board) {

	ASSERT(board!=nullptr);

	// init
	uint_fast64_t key = 0;

	// pieces
	for (int_fast8_t colour = 0; colour < ColourNb; ++colour) {
		uint_fast32_t sq;
		for (const sq_t *ptr = &board->piece[colour][0]; (sq=*ptr) != SquareNone; ++ptr) {
			const uint_fast32_t piece = board->square[sq];
			key ^= hash_piece_key(piece,sq);
		}

		for (const sq_t *ptr = &board->pawn[colour][0]; (sq=*ptr) != SquareNone; ++ptr) {
			const uint_fast32_t piece = board->square[sq];
			key ^= hash_piece_key(piece,sq);
		}
	}

	// castle flags
	key ^= hash_castle_key(board->flags);

	// en-passant square
	const uint_fast32_t sq = board->ep_square;
	if (sq != SquareNone) key ^= hash_ep_key(sq);

   // turn
   key ^= hash_turn_key(board->turn);

   return key;
}

// hash_pawn_key()

uint_fast64_t hash_pawn_key(const board_t * board) {

	ASSERT(board!=nullptr);

	// init
	uint_fast64_t key = 0;

	// pawns
	for (int_fast32_t colour = 0; colour < ColourNb; ++colour) {
		int_fast32_t sq;
		for (const sq_t *ptr = &board->pawn[colour][0]; (sq=*ptr) != SquareNone; ++ptr) {
			const int_fast32_t piece = board->square[sq];
			key ^= hash_piece_key(piece,sq);
		}
	}

   return key;
}

// hash_material_key()

uint_fast64_t hash_material_key(const board_t * board) {

	ASSERT(board!=nullptr);

	// init
	uint_fast64_t key = 0;

	// counters
	for (int_fast32_t piece_12 = 0; piece_12 < 12; ++piece_12) {
		const int_fast32_t count = board->number[piece_12];
		key ^= hash_counter_key(piece_12,count);
	}

	return key;
}

// hash_piece_key()

uint_fast64_t hash_piece_key(int_fast32_t piece, int_fast32_t square) {

	ASSERT(piece_is_ok(piece));
	ASSERT(SQUARE_IS_OK(square));

	return RANDOM_64(RandomPiece+(PIECE_TO_12(piece)^1)*64+SQUARE_TO_64(square)); // HACK: ^1 for PolyGlot book
}

// hash_castle_key()

uint_fast64_t hash_castle_key(int_fast32_t flags) {

	ASSERT((flags&~0xF)==0);
	uint_fast64_t key = 0;

	for (int_fast32_t i = 0; i < 4; ++i)
		if ((flags & (1<<i)) != 0) key ^= RANDOM_64(RandomCastle+i);

	return key;
}

// hash_ep_key()

uint_fast64_t hash_ep_key(int_fast32_t square) {

	ASSERT(SQUARE_IS_OK(square));

	return RANDOM_64(RandomEnPassant+SQUARE_FILE(square)-FileA);
}

// hash_turn_key()

uint_fast64_t hash_turn_key(int_fast32_t colour) {

	ASSERT(COLOUR_IS_OK(colour));

	return (COLOUR_IS_WHITE(colour)) ? RANDOM_64(RandomTurn) : 0;
}

// hash_counter_key()

static uint_fast64_t hash_counter_key(int_fast32_t piece_12, int_fast32_t count) {

	ASSERT(piece_12>=0&&piece_12<12);
	ASSERT(count>=0&&count<=10);

   // init
	uint_fast64_t key = 0;

	// counter

	int_fast32_t index = piece_12 * 16;
	for (int_fast32_t i = 0; i < count; ++i) key ^= RANDOM_64(index+i);

   return key;
}

// end of hash.cpp
