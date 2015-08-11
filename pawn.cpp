
// pawn.cpp

// includes

#include <cstring>

#include "board.h"
#include "colour.h"
#include "hash.h"
#include "option.h"
#include "pawn.h"
#include "piece.h"
#include "protocol.h"
#include "square.h"
#include "util.h"

// constants

static constexpr uint_fast32_t TableSize = 16384; // 256kB

// types

typedef pawn_info_t entry_t;

struct pawn_t {
	int_fast64_t read_nb;
	int_fast64_t read_hit;
	int_fast64_t write_nb;
	int_fast64_t write_collision;
	entry_t * table;
	uint_fast32_t size;
	uint_fast32_t mask;
	uint_fast32_t used;
};

// constants and variables

static /* const */ int_fast32_t PawnStructureWeight = 256; // 100%
/*
static const int_fast32_t DoubledOpening = 10;
static const int_fast32_t DoubledEndgame = 20;

static const int_fast32_t IsolatedOpening = 10;
static const int_fast32_t IsolatedOpeningOpen = 20;
static const int_fast32_t IsolatedEndgame = 20;

static const int_fast32_t BackwardOpening = 8;
static const int_fast32_t BackwardOpeningOpen = 16;
static const int_fast32_t BackwardEndgame = 10;
*/
static constexpr std::array<int_fast32_t, 8> DoubledOpening = {10,10,10,12,12,10,10,10};
static constexpr std::array<int_fast32_t, 8> DoubledEndgame = {18,18,18,20,20,18,18,18};

static constexpr std::array<int_fast32_t, 8> IsolatedOpening =		{ 8, 9,10,12,12,10, 9, 8};
static constexpr std::array<int_fast32_t, 8> IsolatedOpeningOpen =	{18,19,20,22,22,20,19,18};
static constexpr std::array<int_fast32_t, 8> IsolatedEndgame =		{18,19,20,22,22,20,19,18};

static constexpr std::array<int_fast32_t, 8> BackwardOpening =		{ 6, 7, 8,10,10, 8, 7, 6};
static constexpr std::array<int_fast32_t, 8> BackwardOpeningOpen =	{12,14,16,18,18,16,14,12};
static constexpr std::array<int_fast32_t, 8> BackwardEndgame =		{ 8, 9,10,12,12,10, 9, 8};

static constexpr int_fast32_t CandidateOpeningMin = 5;
static constexpr int_fast32_t CandidateOpeningMax = 55;
static constexpr int_fast32_t CandidateEndgameMin = 10;
static constexpr int_fast32_t CandidateEndgameMax = 110;

// this was moved to eval.cpp

/*
static const int_fast32_t PassedOpeningMin = 10;
static const int_fast32_t PassedOpeningMax = 70;
static const int_fast32_t PassedEndgameMin = 20;
static const int_fast32_t PassedEndgameMax = 140;
*/

static /* const */ std::array<int_fast32_t, RankNb> Bonus;

// variables

std::array<int_fast32_t, 16> BitEQ;
std::array<int_fast32_t, 16> BitLT;
std::array<int_fast32_t, 16> BitLE;
std::array<int_fast32_t, 16> BitGT;
std::array<int_fast32_t, 16> BitGE;

std::array<int_fast32_t, 0x100> BitFirst;
std::array<int_fast32_t, 0x100> BitLast;
std::array<int_fast32_t, 0x100> BitCount;
std::array<int_fast32_t, 0x100> BitRev;

static pawn_t Pawn[1];

static std::array<int_fast32_t, RankNb> BitRank1;
static std::array<int_fast32_t, RankNb> BitRank2;
static std::array<int_fast32_t, RankNb> BitRank3;

// prototypes

static void pawn_comp_info (pawn_info_t * info, const board_t * board);

// functions

// pawn_init_bit()

void pawn_init_bit() {

	for (int_fast32_t rank = 0; rank < RankNb; ++rank) {

		BitEQ[rank] = 0;
		BitLT[rank] = 0;
		BitLE[rank] = 0;
		BitGT[rank] = 0;
		BitGE[rank] = 0;

		BitRank1[rank] = 0;
		BitRank2[rank] = 0;
		BitRank3[rank] = 0;
	}

	for (int_fast32_t rank = Rank1; rank <= Rank8; ++rank) {
		BitEQ[rank] = 1 << (rank - Rank1);
		BitLT[rank] = BitEQ[rank] - 1;
		BitLE[rank] = BitLT[rank] | BitEQ[rank];
		BitGT[rank] = BitLE[rank] ^ 0xFF;
		BitGE[rank] = BitGT[rank] | BitEQ[rank];
	}

	for (int_fast32_t rank = Rank1; rank <= Rank8; ++rank) {
		BitRank1[rank] = BitEQ[rank+1];
		BitRank2[rank] = BitEQ[rank+1] | BitEQ[rank+2];
		BitRank3[rank] = BitEQ[rank+1] | BitEQ[rank+2] | BitEQ[rank+3];
	}

   // bit-indexed Bit*[]

	for (int_fast32_t b = 0; b < 0x100; ++b) {

		int_fast32_t first = Rank8; // HACK for pawn shelter
		int_fast32_t  last = Rank1; // HACK
		int_fast32_t  count = 0, rev = 0;

		for (int_fast32_t rank = Rank1; rank <= Rank8; ++rank) {
			if ((b & BitEQ[rank]) != 0) {
				if (rank < first) first = rank;
				if (rank > last) last = rank;
				++count;
				rev |= BitEQ[RANK_OPP(rank)];
			}
		}

		BitFirst[b] = first;
		BitLast[b] = last;
		BitCount[b] = count;
		BitRev[b] = rev;
	}
}

// pawn_init()

void pawn_init() {

	// UCI options

	PawnStructureWeight = (option_get_int("Pawn Structure") * 256 + 50) / 100;

	// bonus

	for (int_fast32_t rank = 0; rank < RankNb; ++rank) Bonus[rank] = 0;

	Bonus[Rank4] = 26;
	Bonus[Rank5] = 77;
	Bonus[Rank6] = 154;
	Bonus[Rank7] = 256;

	// pawn hash-table

	Pawn->size = 0;
	Pawn->mask = 0;
	Pawn->table = nullptr;
}

// pawn_alloc()

void pawn_alloc() {

	ASSERT(sizeof(entry_t)==16);


	Pawn->size = TableSize;
	Pawn->mask = TableSize - 1;
	Pawn->table = (entry_t *) my_malloc(Pawn->size*sizeof(entry_t));

	pawn_clear();

}

// pawn_clear()

void pawn_clear() {

	if (Pawn->table != nullptr)
		memset(Pawn->table,0,Pawn->size*sizeof(entry_t));


	Pawn->used = 0;
	Pawn->read_nb = 0;
	Pawn->read_hit = 0;
	Pawn->write_nb = 0;
	Pawn->write_collision = 0;
}

// pawn_get_info()

void pawn_get_info(pawn_info_t * info, const board_t * board) {

	ASSERT(info!=nullptr);
	ASSERT(board!=nullptr);

	// probe
	Pawn->read_nb++;

	uint_fast64_t key = board->pawn_key;
	entry_t *entry = &Pawn->table[KEY_INDEX(key)&Pawn->mask];

	if (entry->lock == KEY_LOCK(key)) {
		// found
		Pawn->read_hit++;
		*info = *entry;
		return;
	}


	// calculation
	pawn_comp_info(info,board);

	// store
	Pawn->write_nb++;

	if (entry->lock == 0) // HACK: assume free entry
		Pawn->used++;
	else
		Pawn->write_collision++;

	*entry = *info;
	entry->lock = KEY_LOCK(key);
}

// pawn_comp_info()

static void pawn_comp_info(pawn_info_t * info, const board_t * board) {

	ASSERT(info!=nullptr);
	ASSERT(board!=nullptr);

   // pawn_file[]

#if DEBUG
	for (int_fast32_t colour = 0; colour < ColourNb; ++colour) {

		std::array<int_fast32_t, FileNb> pawn_file;
		const int_fast32_t me = colour;

		for (int_fast32_t file = 0; file < FileNb; ++file)
			pawn_file[file] = 0;

		int_fast32_t sq;
		for (const sq_t *ptr = &board->pawn[me][0]; (sq=*ptr) != SquareNone; ++ptr) {

			int_fast32_t file = SQUARE_FILE(sq), rank = PAWN_RANK(sq,me);

			ASSERT(file>=FileA&&file<=FileH);
			ASSERT(rank>=Rank2&&rank<=Rank7);

			pawn_file[file] |= BIT(rank);
		}

		for (int_fast32_t file = 0; file < FileNb; ++file)
			if (board->pawn_file[colour][file] != pawn_file[file]) my_fatal("board->pawn_file[][]\n");

	}
#endif

   // init

   std::array<int_fast32_t, ColourNb> file_bits;
   std::array<int_fast16_t, ColourNb> opening, endgame;
   std::array<uint_fast8_t, ColourNb> flags, passed_bits, single_file, wsp, badpawns;

   opening[0] = opening[1] = endgame[0] = endgame[1] = 0;

   flags[0] = flags[1] = wsp[0] = wsp[1] = badpawns[0] = badpawns[1] = 0;
   file_bits[0] = file_bits[1] = 0;
   passed_bits[0] = passed_bits[1] = 0;
   single_file[0] = single_file[1] = SquareNone;

   // features and scoring

	for (int_fast32_t colour = 0; colour < ColourNb; ++colour) {

		const int_fast32_t me = colour, opp = COLOUR_OPP(me);
		int_fast32_t  sq;
		for (const sq_t *ptr = &board->pawn[me][0]; (sq=*ptr) != SquareNone; ++ptr) {

			// init
			int_fast32_t file = SQUARE_FILE(sq), rank = PAWN_RANK(sq,me);

			ASSERT(file>=FileA&&file<=FileH);
			ASSERT(rank>=Rank2&&rank<=Rank7);

			// flags

			file_bits[me] |= BIT(file);
			if (rank == Rank2) flags[me] |= BackRankFlag;

			// features
			bool backward = false, candidate = false, doubled = false, isolated = false, open = false, passed = false;

			int_fast32_t t1 = board->pawn_file[me][file-1] | board->pawn_file[me][file+1], t2 = board->pawn_file[me][file] | BitRev[board->pawn_file[opp][file]];

			// square colour
			if (SQUARE_COLOUR(sq) == White) ++wsp[me];

			// pawn duo
			if (BIT_COUNT(BitRev[board->pawn_file[me][file+1]]&BitEQ[rank])) {
				opening[me] += 6;
				endgame[me] += 6;
			}

         // doubled

		if ((board->pawn_file[me][file] & BitLT[rank]) != 0)
			doubled = true;


		// isolated and backward

		if (t1 == 0) {

            isolated = true;

		} else if ((t1 & BitLE[rank]) == 0) {

			backward = true;

			// really backward?

			if ((t1 & BitRank1[rank]) != 0) {

				ASSERT(rank+2<=Rank8);

				if (((t2 & BitRank1[rank])
				  | ((BitRev[board->pawn_file[opp][file-1]] | BitRev[board->pawn_file[opp][file+1]]) & BitRank2[rank])) == 0) {
					backward = false;
				}

			} else if (rank == Rank2 && ((t1 & BitEQ[rank+2]) != 0)) {

				ASSERT(rank+3<=Rank8);

				if (((t2 & BitRank2[rank])
				 | ((BitRev[board->pawn_file[opp][file-1]] | BitRev[board->pawn_file[opp][file+1]]) & BitRank3[rank])) == 0) {
					backward = false;
				}
			}
		}

		// open, candidate and passed

		if ((t2 & BitGT[rank]) == 0) {

			open = true;

			if (((BitRev[board->pawn_file[opp][file-1]] | BitRev[board->pawn_file[opp][file+1]]) & BitGT[rank]) == 0) {

				passed = true;
				passed_bits[me] |= BIT(file);

			} else {

				// candidate?

				int_fast32_t n = 0;

				n += BIT_COUNT(board->pawn_file[me][file-1]&BitLE[rank]);
				n += BIT_COUNT(board->pawn_file[me][file+1]&BitLE[rank]);

				n -= BIT_COUNT(BitRev[board->pawn_file[opp][file-1]]&BitGT[rank]);
				n -= BIT_COUNT(BitRev[board->pawn_file[opp][file+1]]&BitGT[rank]);

				if (n >= 0) {

					// safe?

					int_fast32_t n2 = 0;

					n2 += BIT_COUNT(board->pawn_file[me][file-1]&BitEQ[rank-1]);
					n2 += BIT_COUNT(board->pawn_file[me][file+1]&BitEQ[rank-1]);

					n2 -= BIT_COUNT(BitRev[board->pawn_file[opp][file-1]]&BitEQ[rank+1]);
					n2 -= BIT_COUNT(BitRev[board->pawn_file[opp][file+1]]&BitEQ[rank+1]);

					if (n2 >= 0) candidate = true;
				}
			}
		}

		// score

		if (doubled) {
			opening[me] -= DoubledOpening[file-FileA];
			endgame[me] -= DoubledEndgame[file-FileA];
		}

		if (isolated) {
			if (open) {
				opening[me] -= IsolatedOpeningOpen[file-FileA];
				endgame[me] -= IsolatedEndgame[file-FileA];

				switch (file) {
					case FileA: badpawns[me] |= BadPawnFileA; break;
					case FileB: badpawns[me] |= BadPawnFileB; break;
					case FileC: badpawns[me] |= BadPawnFileC; break;
					case FileD: badpawns[me] |= BadPawnFileD; break;
					case FileE: badpawns[me] |= BadPawnFileE; break;
					case FileF: badpawns[me] |= BadPawnFileF; break;
					case FileG: badpawns[me] |= BadPawnFileG; break;
					case FileH: badpawns[me] |= BadPawnFileH; break;
					}
			} else {
				opening[me] -= IsolatedOpening[file-FileA];
				endgame[me] -= IsolatedEndgame[file-FileA];
			}
		}

		if (backward) {
			if (open) {
				opening[me] -= BackwardOpeningOpen[file-FileA];
				endgame[me] -= BackwardEndgame[file-FileA];

				switch (file) {
				case FileA: badpawns[me] |= BadPawnFileA; break;
				case FileB: badpawns[me] |= BadPawnFileB; break;
				case FileC: badpawns[me] |= BadPawnFileC; break;
				case FileD: badpawns[me] |= BadPawnFileD; break;
				case FileE: badpawns[me] |= BadPawnFileE; break;
				case FileF: badpawns[me] |= BadPawnFileF; break;
				case FileG: badpawns[me] |= BadPawnFileG; break;
				case FileH: badpawns[me] |= BadPawnFileH; break;
				}
			} else {
				opening[me] -= BackwardOpening[file-FileA];
				endgame[me] -= BackwardEndgame[file-FileA];
			}
		}

		if (candidate) {
			opening[me] += quad(CandidateOpeningMin,CandidateOpeningMax,rank);
			endgame[me] += quad(CandidateEndgameMin,CandidateEndgameMax,rank);
		}

		// this was moved to the dynamic evaluation

/*
		if (passed) {
			opening[me] += quad(PassedOpeningMin,PassedOpeningMax,rank);
			endgame[me] += quad(PassedEndgameMin,PassedEndgameMax,rank);
		}
*/
		}
	}

	// store info

	info->opening = ((opening[White] - opening[Black]) * PawnStructureWeight) / 256;
	info->endgame = ((endgame[White] - endgame[Black]) * PawnStructureWeight) / 256;

	int_fast32_t bits_w = file_bits[0];

	if (bits_w != 0 && (bits_w & (bits_w-1)) == 0) { // one set bit

		const int_fast32_t file = BIT_FIRST(bits_w);
		int_fast32_t rank = BIT_FIRST(board->pawn_file[0][file]);
		ASSERT(rank>=Rank2);

		if (((BitRev[board->pawn_file[1][file-1]] | BitRev[board->pawn_file[1][file+1]]) & BitGT[rank]) == 0) {
			rank = BIT_LAST(board->pawn_file[0][file]);
			single_file[0] = SQUARE_MAKE(file,rank);
		}
	}


	int_fast32_t bits_b = file_bits[1];

	if (bits_b != 0 && (bits_b & (bits_b-1)) == 0) { // one set bit

		const int_fast32_t file = BIT_FIRST(bits_b);
		int_fast32_t rank = BIT_FIRST(board->pawn_file[1][file]);
		ASSERT(rank>=Rank2);

		if (((BitRev[board->pawn_file[0][file-1]] | BitRev[board->pawn_file[0][file+1]]) & BitGT[rank]) == 0) {
			rank = BIT_LAST(board->pawn_file[1][file]);
			single_file[1] = SQUARE_MAKE(file,rank);
		}
	}

	info->flags[0] = flags[0];
	info->flags[1] = flags[1];
	info->passed_bits[0] = passed_bits[0];
	info->passed_bits[1] = passed_bits[1];
	info->single_file[0] = single_file[0];
	info->single_file[1] = single_file[1];
	info->badpawns[0] = badpawns[0];
	info->badpawns[1] = badpawns[1];
	info->wsp[0] = wsp[0];
	info->wsp[1] = wsp[1];

}

// quad()

int_fast32_t quad(int_fast32_t y_min, int_fast32_t y_max, int_fast32_t x) {

	ASSERT(y_min>=0&&y_min<=y_max&&y_max<=+32767);
	ASSERT(x>=Rank2&&x<=Rank7);

	int_fast32_t y = y_min + ((y_max - y_min) * Bonus[x] + 128) / 256;
	ASSERT(y>=y_min&&y<=y_max);

	return y;
}

// end of pawn.cpp