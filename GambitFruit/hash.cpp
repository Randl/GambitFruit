// hash.cpp

// includes

#include "board.h"
#include "hash.h"
#include "random.h"


// variables

std::array<U64, 16> Castle64;

// prototypes

static U64 hash_counter_key(S32 piece_12, S32 count);

// functions

// hash_init()

void hash_init() {
	for (U8 i = 0; i < 16; ++i)
		Castle64[i] = hash_castle_key(i);
}

// hash_key()

U64 hash_key(const board_t *board) {

	ASSERT(board != nullptr);

	// init
	U64 key = 0;

	// pieces
	for (S8 colour = 0; colour < ColourNb; ++colour) {
		for (auto sq = board->piece[colour].begin(); sq != board->piece[colour].end(); ++sq) {
			const S32 piece = board->square[*sq];
			key ^= hash_piece_key(piece, *sq);
		}

		for (auto sq = board->pawn[colour].begin(); sq != board->pawn[colour].end(); ++sq) {
			const S32 piece = board->square[*sq];
			key ^= hash_piece_key(piece, *sq);
		}
	}

	// castle flags
	key ^= hash_castle_key(board->flags);

	// en-passant square
	const S32 sq = board->ep_square;
	if (sq != SquareNone) key ^= hash_ep_key(sq);

	// turn
	key ^= hash_turn_key(board->turn);

	return key;
}

// hash_pawn_key()

U64 hash_pawn_key(const board_t *board) {

	ASSERT(board != nullptr);

	// init
	U64 key = 0;

	// pawns
	for (S8 colour = 0; colour < ColourNb; ++colour) {
		for (auto sq = board->pawn[colour].begin(); sq != board->pawn[colour].end(); ++sq) {
			const S32 piece = board->square[*sq];
			key ^= hash_piece_key(piece, *sq);
		}
	}

	return key;
}

// hash_material_key()

U64 hash_material_key(const board_t *board) {

	ASSERT(board != nullptr);

	// init
	U64 key = 0;

	// counters
	for (U8 piece_12 = 0; piece_12 < 12; ++piece_12) {
		const S32 count = board->number[piece_12];
		key ^= hash_counter_key(piece_12, count);
	}

	return key;
}

// hash_piece_key()

U64 hash_piece_key(S32 piece, S32 square) {

	ASSERT(piece_is_ok(piece));
	ASSERT(SQUARE_IS_OK(square));

	return RANDOM_64(RandomPiece + (PIECE_TO_12(piece) ^ 1) * 64 + SQUARE_TO_64(square)); // HACK: ^1 for PolyGlot book
}

// hash_castle_key()

U64 hash_castle_key(S32 flags) {

	ASSERT((flags & ~0xF) == 0);
	U64 key = 0;

	for (U8 i = 0; i < 4; ++i)
		if ((flags & (1 << i)) != 0) key ^= RANDOM_64(RandomCastle + i);

	return key;
}

// hash_ep_key()

U64 hash_ep_key(S32 square) {

	ASSERT(SQUARE_IS_OK(square));

	return RANDOM_64(RandomEnPassant + SQUARE_FILE(square) - FileA);
}

// hash_turn_key()

U64 hash_turn_key(S8 colour) {

	ASSERT(COLOUR_IS_OK(colour));

	return (COLOUR_IS_WHITE(colour)) ? RANDOM_64(RandomTurn) : 0;
}

// hash_counter_key()

static U64 hash_counter_key(S32 piece_12, S32 count) {

	ASSERT(piece_12 >= 0 && piece_12 < 12);
	ASSERT(count >= 0 && count <= 10);

	// init
	U64 key = 0;

	// counter

	S32 index = piece_12 * 16;
	for (U8 i = 0; i < count; ++i) key ^= RANDOM_64(index + i);

	return key;
}

// end of hash.cpp
