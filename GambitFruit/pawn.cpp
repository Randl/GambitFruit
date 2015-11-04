// pawn.cpp

// includes

#include <cstring>

#include "board.h"
#include "hash.h"
#include "option.h"
#include "pawn.h"

// constants

static constexpr U16 TableSize = 25600; // ~512kB    //16384; // 256kB

// types

typedef pawn_info_t entry_t;

struct pawn_t {
    S64 read_nb;
    S64 read_hit;
    S64 write_nb;
    S64 write_collision;
    entry_t *table;
    U32 size;
    U32 mask;
    U32 used;
};

// constants and variables

static /* const */ S32 PawnStructureWeight = 256; // 100%

static /*constexpr*/ std::array<S8, 8> DoubledOpening = {10, 10, 10, 12, 12, 10, 10, 10};
static /*constexpr*/ std::array<S8, 8> DoubledEndgame = {18, 18, 18, 20, 20, 18, 18, 18};

static /*constexpr*/ std::array<S8, 8> IsolatedOpening = {8, 9, 10, 12, 12, 10, 9, 8};
static /*constexpr*/ std::array<S8, 8> IsolatedOpeningOpen = {18, 19, 20, 22, 22, 20, 19, 18};
static /*constexpr*/ std::array<S8, 8> IsolatedEndgame = {18, 19, 20, 22, 22, 20, 19, 18};

static /*constexpr*/ std::array<S8, 8> BackwardOpening = {6, 7, 8, 10, 10, 8, 7, 6};
static /*constexpr*/ std::array<S8, 8> BackwardOpeningOpen = {12, 14, 16, 18, 18, 16, 14, 12};
static /*constexpr*/ std::array<S8, 8> BackwardEndgame = {8, 9, 10, 12, 12, 10, 9, 8};

static constexpr S8 CandidateOpeningMin = 5;
static constexpr S16 CandidateOpeningMax = 55;
static constexpr S8 CandidateEndgameMin = 10;
static constexpr S16 CandidateEndgameMax = 110;

// this was moved to eval.cpp

/*
static const S32 PassedOpeningMin = 10;
static const S32 PassedOpeningMax = 70;
static const S32 PassedEndgameMin = 20;
static const S32 PassedEndgameMax = 140;
*/

static /* const */ std::array<S32, RankNb> Bonus;

// variables

std::array<S32, 16> BitEQ;
std::array<S32, 16> BitLT;
std::array<S32, 16> BitLE;
std::array<S32, 16> BitGT;
std::array<S32, 16> BitGE;

std::array<S32, 0x100> BitFirst;
std::array<S32, 0x100> BitLast;
std::array<S32, 0x100> BitCount;
std::array<S32, 0x100> BitRev;

static pawn_t Pawn[1];

static std::array<S32, RankNb> BitRank1;
static std::array<S32, RankNb> BitRank2;
static std::array<S32, RankNb> BitRank3;

// prototypes

static void pawn_comp_info(pawn_info_t *info, const board_t *board);

// functions

// pawn_init_bit()

void pawn_init_bit() {

	for (U8 rank = 0; rank < RankNb; ++rank) {

		BitEQ[rank] = 0;
		BitLT[rank] = 0;
		BitLE[rank] = 0;
		BitGT[rank] = 0;
		BitGE[rank] = 0;

		BitRank1[rank] = 0;
		BitRank2[rank] = 0;
		BitRank3[rank] = 0;
	}

	for (U16 rank = Rank1; rank <= Rank8; ++rank) {
		BitEQ[rank] = 1 << (rank - Rank1);
		BitLT[rank] = BitEQ[rank] - 1;
		BitLE[rank] = BitLT[rank] | BitEQ[rank];
		BitGT[rank] = BitLE[rank] ^ 0xFF;
		BitGE[rank] = BitGT[rank] | BitEQ[rank];
	}

	for (U16 rank = Rank1; rank <= Rank8; ++rank) {
		BitRank1[rank] = BitEQ[rank + 1];
		BitRank2[rank] = BitEQ[rank + 1] | BitEQ[rank + 2];
		BitRank3[rank] = BitEQ[rank + 1] | BitEQ[rank + 2] | BitEQ[rank + 3];
	}

	// bit-indexed Bit*[]

	for (U16 b = 0; b < 0x100; ++b) {

		S32 first = Rank8; // HACK for pawn shelter
		S32 last = Rank1; // HACK
		S32 count = 0, rev = 0;

		for (U16 rank = Rank1; rank <= Rank8; ++rank) {
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
#ifdef TMO_OPT
	char dop[] = "DoubledOpening0", deg[] = "DoubledEndgame0", iso[] = "IsolatedOpening0",
		isoo[] = "IsolatedOpeningOpen0", ise[] = "IsolatedEndgame0", bo[] = "BackwardOpening0",
		boo[] = "BackwardOpeningOpen0", beg[] = "BackwardEndgame0";
	for (U8 i = 0; i < 8; ++i) {
		dop[14]++;
		deg[14]++;
		iso[15]++;
		isoo[19]++;
		ise[15]++;
		bo[15]++;
		boo[19]++;
		beg[15]++;
		DoubledOpening[i] = option_get_int(dop);
		DoubledEndgame[i] = option_get_int(deg);
		IsolatedOpening[i] = option_get_int(iso);
		IsolatedOpeningOpen[i] = option_get_int(isoo);
		IsolatedEndgame[i] = option_get_int(ise);
		BackwardOpening[i] = option_get_int(bo);
		BackwardOpeningOpen[i] = option_get_int(boo);
		BackwardEndgame[i] = option_get_int(beg);
	}
#endif

	// bonus

	for (U8 rank = 0; rank < RankNb; ++rank) Bonus[rank] = 0;

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

	ASSERT(sizeof(entry_t) == 20); //not anymore
	//printf("sizeof(entry_t) == %llu\n", sizeof(entry_t));
	Pawn->size = TableSize;
	Pawn->mask = TableSize - 1;
	Pawn->table = (entry_t *) my_malloc(Pawn->size * sizeof(entry_t));
//    Pawn->table = new entry_t[Pawn->size];

	pawn_clear();

}

// pawn_clear()

void pawn_clear() {

	if (Pawn->table != nullptr)
		memset(Pawn->table, 0, Pawn->size * sizeof(entry_t));

	Pawn->used = 0;
	Pawn->read_nb = 0;
	Pawn->read_hit = 0;
	Pawn->write_nb = 0;
	Pawn->write_collision = 0;
}

// pawn_get_info()

void pawn_get_info(pawn_info_t *info, const board_t *board) {

	ASSERT(info != nullptr);
	ASSERT(board != nullptr);

	// probe
	Pawn->read_nb++;

	U64 key = board->pawn_key;
	entry_t *entry = &Pawn->table[KEY_INDEX(key) & Pawn->mask];

	if (entry->lock == KEY_LOCK(key)) {
		// found
		Pawn->read_hit++;
		*info = *entry;
		return;
	}


	// calculation
	pawn_comp_info(info, board);

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

static void pawn_comp_info(pawn_info_t *info, const board_t *board) {

	ASSERT(info != nullptr);
	ASSERT(board != nullptr);

	// pawn_file[]

#ifdef DEBUG
	for (S8 colour = 0; colour < ColourNb; ++colour) {

		std::array<S32, FileNb> pawn_file;
		const S8 me = colour;

		for (U8 file = 0; file < FileNb; ++file)
			pawn_file[file] = 0;

		for (auto sq = board->pawn[me].begin(); sq != board->pawn[me].end(); ++sq) {

			S32 file = SQUARE_FILE(*sq), rank = PAWN_RANK(*sq, me);

			ASSERT(file >= FileA && file <= FileH);
			ASSERT(rank >= Rank2 && rank <= Rank7);

			pawn_file[file] |= BIT(rank);
		}

		for (U8 file = 0; file < FileNb; ++file)
			if (board->pawn_file[colour][file] != pawn_file[file]) my_fatal("board->pawn_file[][]\n");

	}
#endif

	// init

	std::array<S32, ColourNb> file_bits;
	std::array<S16, ColourNb> opening, endgame;
	std::array<U8, ColourNb> flags, passed_bits, single_file, wsp, badpawns;

	opening[0] = opening[1] = endgame[0] = endgame[1] = 0;

	flags[0] = flags[1] = wsp[0] = wsp[1] = badpawns[0] = badpawns[1] = 0;
	file_bits[0] = file_bits[1] = 0;
	passed_bits[0] = passed_bits[1] = 0;
	single_file[0] = single_file[1] = SquareNone;

	// features and scoring

	for (S8 colour = 0; colour < ColourNb; ++colour) {

		const S8 me = colour, opp = COLOUR_OPP(me);

		for (auto sq = board->pawn[me].begin(); sq != board->pawn[me].end(); ++sq) {

			// init
			S32 file = SQUARE_FILE(*sq), rank = PAWN_RANK(*sq, me);

			ASSERT(file >= FileA && file <= FileH);
			ASSERT(rank >= Rank2 && rank <= Rank7);

			// flags

			file_bits[me] |= BIT(file);
			if (rank == Rank2) flags[me] |= BackRankFlag;

			// features
			bool backward = false, candidate = false, doubled = false, isolated = false, open = false;// ram = false;

			S32 neighbor_files = board->pawn_file[me][file - 1] | board->pawn_file[me][file + 1],
				all_file_pawns = board->pawn_file[me][file] | BitRev[board->pawn_file[opp][file]];

			// square colour
			if (SQUARE_COLOUR(*sq) == White) ++wsp[me];

			// pawn duo
			if (BIT_COUNT(BitRev[board->pawn_file[me][file + 1]] & BitEQ[rank])) {
				opening[me] += 6;
				endgame[me] += 6;
			}

			// doubled

			if ((board->pawn_file[me][file] & BitLT[rank]) != 0)
				doubled = true;

			/* //TODO: test
			//pawn ram
			if ((board->pawn_file[me][file] << 1) | BitRev[board->pawn_file[opp][file]] == 0)  {
				opening += PawnRamOpening[rank];
				endgame += PawnRamEndgame[rank];
				pawn_ram_bits |= file;
			}
			*/

			// isolated and backward

			if (neighbor_files == 0) {

				isolated = true;

			} else if ((neighbor_files & BitLE[rank]) == 0) {

				backward = true;

				// really backward?

				if ((neighbor_files & BitRank1[rank]) != 0) {

					ASSERT(rank + 2 <= Rank8);

					if (((all_file_pawns & BitRank1[rank])
						| ((BitRev[board->pawn_file[opp][file - 1]] | BitRev[board->pawn_file[opp][file + 1]])
							& BitRank2[rank])) == 0) {
						backward = false;
					}

				} else if (rank == Rank2 && ((neighbor_files & BitEQ[rank + 2]) != 0)) {

					ASSERT(rank + 3 <= Rank8);

					if (((all_file_pawns & BitRank2[rank])
						| ((BitRev[board->pawn_file[opp][file - 1]] | BitRev[board->pawn_file[opp][file + 1]])
							& BitRank3[rank])) == 0) {
						backward = false;
					}
				}
			}

			// open, candidate and passed

			if ((all_file_pawns & BitGT[rank]) == 0) {

				open = true;

				if (((BitRev[board->pawn_file[opp][file - 1]] | BitRev[board->pawn_file[opp][file + 1]]) & BitGT[rank])
					== 0) {

					//passed = true;
					passed_bits[me] |= BIT(file);

				} else {

					// candidate?

					S32 n = 0;

					n += BIT_COUNT(board->pawn_file[me][file - 1] & BitLE[rank]);
					n += BIT_COUNT(board->pawn_file[me][file + 1] & BitLE[rank]);

					n -= BIT_COUNT(BitRev[board->pawn_file[opp][file - 1]] & BitGT[rank]);
					n -= BIT_COUNT(BitRev[board->pawn_file[opp][file + 1]] & BitGT[rank]);

					if (n >= 0) {

						// safe?

						S32 n2 = 0;

						n2 += BIT_COUNT(board->pawn_file[me][file - 1] & BitEQ[rank - 1]);
						n2 += BIT_COUNT(board->pawn_file[me][file + 1] & BitEQ[rank - 1]);

						n2 -= BIT_COUNT(BitRev[board->pawn_file[opp][file - 1]] & BitEQ[rank + 1]);
						n2 -= BIT_COUNT(BitRev[board->pawn_file[opp][file + 1]] & BitEQ[rank + 1]);

						if (n2 >= 0) candidate = true;
					}
				}
			}

			// score

			if (doubled) {
				opening[me] -= DoubledOpening[file - FileA];
				endgame[me] -= DoubledEndgame[file - FileA];
			}

			if (isolated) {
				if (open) {
					opening[me] -= IsolatedOpeningOpen[file - FileA];
					endgame[me] -= IsolatedEndgame[file - FileA];

					const S32 BadPawnFile = 1 << (file - FileA); // HACK: see BadPawnFileA and FileA
					badpawns[me] |= BadPawnFile;

				} else {
					opening[me] -= IsolatedOpening[file - FileA];
					endgame[me] -= IsolatedEndgame[file - FileA];
				}
			}

			if (backward) {
				if (open) {
					opening[me] -= BackwardOpeningOpen[file - FileA];
					endgame[me] -= BackwardEndgame[file - FileA];

					const S32 BadPawnFile = 1 << (file - FileA); // HACK: see BadPawnFileA and FileA
					badpawns[me] |= BadPawnFile;

				} else {
					opening[me] -= BackwardOpening[file - FileA];
					endgame[me] -= BackwardEndgame[file - FileA];
				}
			}

			if (candidate) {
				opening[me] += quad(CandidateOpeningMin, CandidateOpeningMax, rank);
				endgame[me] += quad(CandidateEndgameMin, CandidateEndgameMax, rank);
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

	S32 bits_w = file_bits[0];

	if (bits_w != 0 && (bits_w & (bits_w - 1)) == 0) { // one set bit

		const S32 file = BIT_FIRST(bits_w);
		S32 rank = BIT_FIRST(board->pawn_file[0][file]);
		ASSERT(rank >= Rank2);

		if (((BitRev[board->pawn_file[1][file - 1]] | BitRev[board->pawn_file[1][file + 1]]) & BitGT[rank]) == 0) {
			rank = BIT_LAST(board->pawn_file[0][file]);
			single_file[0] = SQUARE_MAKE(file, rank);
		}
	}

	S32 bits_b = file_bits[1];

	if (bits_b != 0 && (bits_b & (bits_b - 1)) == 0) { // one set bit

		const S32 file = BIT_FIRST(bits_b);
		S32 rank = BIT_FIRST(board->pawn_file[1][file]);
		ASSERT(rank >= Rank2);

		if (((BitRev[board->pawn_file[0][file - 1]] | BitRev[board->pawn_file[0][file + 1]]) & BitGT[rank]) == 0) {
			rank = BIT_LAST(board->pawn_file[1][file]);
			single_file[1] = SQUARE_MAKE(file, rank);
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

S32 quad(S32 y_min, S32 y_max, S32 x) {

	ASSERT(y_min >= 0 && y_min <= y_max && y_max <= +32767);
	ASSERT(x >= Rank2 && x <= Rank7);

	S32 y = y_min + ((y_max - y_min) * Bonus[x] + 128) / 256;
	ASSERT(y >= y_min && y <= y_max);

	return y;
}

// end of pawn.cpp
