// material.cpp

// includes

#include <cstring>
#include <array>
#include "board.h"
#include "hash.h"
#include "material.h"
#include "option.h"

// constants
static constexpr U16 TableSize = 256; // 4kB

static constexpr U8 PawnPhase = 0;
static constexpr U8 KnightPhase = 1;
static constexpr U8 BishopPhase = 1;
static constexpr U8 RookPhase = 2;
static constexpr U8 QueenPhase = 4;

static constexpr S16 TotalPhase = PawnPhase * 16 + KnightPhase * 4 + BishopPhase * 4 + RookPhase * 4 + QueenPhase * 2;

// constants and variables

//static /* const */ S32 MaterialWeight = 256; // 100%

static /*const*/ S32 PawnOpening = 80; // was 100
static /*const*/ S32 PawnEndgame = 90; // was 100
static /*const*/ S32 KnightOpening = 320;
static /*const*/ S32 KnightEndgame = 320;
static /*const*/ S32 BishopOpening = 325;
static /*const*/ S32 BishopEndgame = 325;
static /*const*/ S32 RookOpening = 500;
static /*const*/ S32 RookEndgame = 500;
static /*const*/ S32 QueenOpening = 975;
static /*const*/ S32 QueenEndgame = 975;

static /*const*/ S32 BishopPairOpening = 50;
static /*const*/ S32 BishopPairEndgame = 50;

static /*const*/ S32 Queen_Knight_combo = 15; // with no rooks
static /*const*/ S32 Rook_Bishop_combo = 15;  // with no queens
static /*const*/ S32 bad_trade_value = 50; // not used like in crafty... (will be for 3 minors vs queen)

static constexpr std::array<S8, 17>
	RookPawnPenalty = {15, 15, 13, 11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9, -11, -13, -15};

// types

typedef material_info_t entry_t;

struct material_t {
    S64 read_nb;
    S64 read_hit;
    S64 write_nb;
    S64 write_collision;
    entry_t *table;
    U32 size;
    U32 mask;
    U32 used;
};

// variables

entry_t material_table[TableSize];

static material_t Material[1];

U8 bitbase_pieces = 2;

// prototypes

static void material_comp_info(material_info_t *info, const board_t *board);

// functions

// material_init()

void material_init() {

	// UCI options

	bitbase_pieces = (option_get_int("Scorpio Bitbases Pieces"));

	//MaterialWeight = (option_get_int("Material") * 256 + 50) / 100;
	bad_trade_value = option_get_int("Bad Trade Value");

	BishopPairOpening = option_get_int("Bishop Pair Opening");
	BishopPairEndgame = option_get_int("Bishop Pair Endgame");

	Queen_Knight_combo = option_get_int("Queen Knight combo");
	Rook_Bishop_combo = option_get_int("Rook Bishop combo");

	PawnOpening = option_get_int("Opening Pawn Value");
	KnightOpening = option_get_int("Opening Knight Value");
	BishopOpening = option_get_int("Opening Bishop Value");
	RookOpening = option_get_int("Opening Rook Value");
	QueenOpening = option_get_int("Opening Queen Value");

	PawnEndgame = option_get_int("Endgame Pawn Value");
	KnightEndgame = option_get_int("Endgame Knight Value");
	BishopEndgame = option_get_int("Endgame Bishop Value");
	RookEndgame = option_get_int("Endgame Rook Value");
	QueenEndgame = option_get_int("Endgame Queen Value");

	// material table
	Material->size = 0;
	Material->mask = 0;
	Material->table = nullptr;
}

// material_alloc()

void material_alloc() {

	ASSERT(sizeof(entry_t) == 16);

	Material->size = TableSize;
	Material->mask = TableSize - 1;
	//Material->table = (entry_t *) my_malloc(Material->size * sizeof(entry_t));
	Material->table = material_table;
//    Material->table = new entry_t[Material->size];
	material_clear();

}

// material_clear()

void material_clear() {

	if (Material->table != nullptr)
		memset(Material->table, 0, Material->size * sizeof(entry_t));

	Material->used = 0;
	Material->read_nb = 0;
	Material->read_hit = 0;
	Material->write_nb = 0;
	Material->write_collision = 0;
}

// material_get_info()

void material_get_info(material_info_t *info, const board_t *board) {

	ASSERT(info != nullptr);
	ASSERT(board != nullptr);

	// probe
	Material->read_nb++;

	U64 key = board->material_key;
	entry_t *entry = &Material->table[KEY_INDEX(key) & Material->mask];

	if (entry->lock == KEY_LOCK(key)) {
		// found
		Material->read_hit++;
		*info = *entry;
		return;
	}

	// calculation
	material_comp_info(info, board);

	// store
	Material->write_nb++;

	if (entry->lock == 0)  // HACK: assume free entry
		Material->used++;
	else
		Material->write_collision++;

	*entry = *info;
	entry->lock = KEY_LOCK(key);
}

// material_comp_info()

static void material_comp_info(material_info_t *info, const board_t *board) {

	ASSERT(info != nullptr);
	ASSERT(board != nullptr);

	// init
	S32 wp, wn, wb, wr, wq;
	S32 bp, bn, bb, br, bq;
	S32 wt, bt;
	S32 wm, bm;

	wp = board->number[WhitePawn12];
	wn = board->number[WhiteKnight12];
	wb = board->number[WhiteBishop12];
	wr = board->number[WhiteRook12];
	wq = board->number[WhiteQueen12];

	bp = board->number[BlackPawn12];
	bn = board->number[BlackKnight12];
	bb = board->number[BlackBishop12];
	br = board->number[BlackRook12];
	bq = board->number[BlackQueen12];

	wt = wq + wr + wb + wn + wp; // no king
	bt = bq + br + bb + bn + bp; // no king

	wm = wb + wn;
	bm = bb + bn;

	// recogniser
	U8 recog = MAT_NONE;

	if (false) {
	} /*else if (wt == 0 && bt == 0) {
		recog = MAT_KK;
	}*/ else if (wt == 2 && bt == 1) {
		if (wr == 1 && wp == 1 && br == 1) recog = MAT_KRPKR;
		if (wb == 1 && wp == 1 && bb == 1) recog = MAT_KBPKB;
	} else if (wt == 1 && bt == 2) {
		if (wr == 1 && br == 1 && bp == 1) recog = MAT_KRKRP;
		if (wb == 1 && bb == 1 && bp == 1) recog = MAT_KBKBP;
	}

	// draw node (exact-draw recogniser)
	U8 flags = 0; // TODO: MOVE ME
	std::array<U8, ColourNb> cflags;
	cflags[0] = cflags[1] = 0;

	// bishop endgame
	if (wq + wr + wn == 0 && bq + br + bn == 0)  // only bishops
	if (wb == 1 && bb == 1) if (wp - bp >= -2 && wp - bp <= +2)  // pawn diff <= 2
		flags |= DrawBishopFlag;

	// multipliers
	std::array<U8, ColourNb> mul;
	mul[0] = mul[1] = 16; // 1

	// white multiplier
	if (wp == 0) { // white has no pawns
		S32 w_maj = wq * 2 + wr;
		S32 w_min = wb + wn;
		S32 w_tot = w_maj * 2 + w_min;

		S32 b_maj = bq * 2 + br;
		S32 b_min = bb + bn;
		S32 b_tot = b_maj * 2 + b_min;

		if (false) {
		} else if (w_tot == 1) {
			ASSERT(w_maj == 0);
			ASSERT(w_min == 1);
			// KBK* or KNK*, always insufficient
			mul[White] = 0;
		} else if (w_tot == 2 && wn == 2) {
			ASSERT(w_maj == 0);
			ASSERT(w_min == 2);
			// KNNK*, usually insufficient
			if (b_tot != 0 || bp == 0)
				mul[White] = 0;
			else // KNNKP+, might not be draw
				mul[White] = 1; // 1/16
		} else if (w_tot == 2 && wb == 2 && b_tot == 1 && bn == 1) {
			ASSERT(w_maj == 0);
			ASSERT(w_min == 2);
			ASSERT(b_maj == 0);
			ASSERT(b_min == 1);
			// KBBKN*, barely drawish (not at all?)
			// TODO: check in TB
			mul[White] = 8; // 1/2
		} else if (w_tot - b_tot <= 1 && w_maj <= 2) {
			// no more than 1 minor up, drawish
			mul[White] = 2; // 1/8
		}

	} else if (wp == 1) { // white has one pawn
		S32 w_maj = wq * 2 + wr;
		S32 w_min = wb + wn;
		S32 w_tot = w_maj * 2 + w_min;

		S32 b_maj = bq * 2 + br;
		S32 b_min = bb + bn;
		S32 b_tot = b_maj * 2 + b_min;

		if (false) {
		} else if (b_min != 0) {
			// assume black sacrifices a minor against the lone pawn
			--b_min;
			--b_tot;
			if (false) {
			} else if (w_tot == 1) {
				ASSERT(w_maj == 0);
				ASSERT(w_min == 1);

				// KBK* or KNK*, always insufficient
				mul[White] = 4; // 1/4
			} else if (w_tot == 2 && wn == 2) {
				ASSERT(w_maj == 0);
				ASSERT(w_min == 2);
				// KNNK*, usually insufficient
				mul[White] = 4; // 1/4
			} else if (w_tot - b_tot <= 1 && w_maj <= 2) {
				// no more than 1 minor up, drawish
				mul[White] = 8; // 1/2
			}
		} else if (br != 0) {
			// assume black sacrifices a rook against the lone pawn
			--b_maj;
			b_tot -= 2;

			if (false) {
			} else if (w_tot == 1) {
				ASSERT(w_maj == 0);
				ASSERT(w_min == 1);
				// KBK* or KNK*, always insufficient
				mul[White] = 4; // 1/4
			} else if (w_tot == 2 && wn == 2) {
				ASSERT(w_maj == 0);
				ASSERT(w_min == 2);
				// KNNK*, usually insufficient
				mul[White] = 4; // 1/4
			} else if (w_tot - b_tot <= 1 && w_maj <= 2) {
				// no more than 1 minor up, drawish
				mul[White] = 8; // 1/2
			}
		}
	}

	// black multiplier

	if (bp == 0) { // black has no pawns

		S32 w_maj = wq * 2 + wr;
		S32 w_min = wb + wn;
		S32 w_tot = w_maj * 2 + w_min;

		S32 b_maj = bq * 2 + br;
		S32 b_min = bb + bn;
		S32 b_tot = b_maj * 2 + b_min;

		if (false) {
		} else if (b_tot == 1) {
			ASSERT(b_maj == 0);
			ASSERT(b_min == 1);

			// KBK* or KNK*, always insufficient
			mul[Black] = 0;
		} else if (b_tot == 2 && bn == 2) {
			ASSERT(b_maj == 0);
			ASSERT(b_min == 2);
			// KNNK*, usually insufficient
			if (w_tot != 0 || wp == 0)
				mul[Black] = 0;
			else  // KNNKP+, might not be draw
				mul[Black] = 1; // 1/16

		} else if (b_tot == 2 && bb == 2 && w_tot == 1 && wn == 1) {
			ASSERT(b_maj == 0);
			ASSERT(b_min == 2);
			ASSERT(w_maj == 0);
			ASSERT(w_min == 1);

			// KBBKN*, barely drawish (not at all?)
			mul[Black] = 8; // 1/2
		} else if (b_tot - w_tot <= 1 && b_maj <= 2) {
			// no more than 1 minor up, drawish
			mul[Black] = 2; // 1/8
		}
	} else if (bp == 1) { // black has one pawn

		S32 w_maj = wq * 2 + wr;
		S32 w_min = wb + wn;
		S32 w_tot = w_maj * 2 + w_min;

		S32 b_maj = bq * 2 + br;
		S32 b_min = bb + bn;
		S32 b_tot = b_maj * 2 + b_min;

		if (false) {
		} else if (w_min != 0) {
			// assume white sacrifices a minor against the lone pawn
			--w_min;
			--w_tot;
			if (false) {
			} else if (b_tot == 1) {
				ASSERT(b_maj == 0);
				ASSERT(b_min == 1);
				// KBK* or KNK*, always insufficient
				mul[Black] = 4; // 1/4
			} else if (b_tot == 2 && bn == 2) {
				ASSERT(b_maj == 0);
				ASSERT(b_min == 2);
				// KNNK*, usually insufficient
				mul[Black] = 4; // 1/4
			} else if (b_tot - w_tot <= 1 && b_maj <= 2) {
				// no more than 1 minor up, drawish
				mul[Black] = 8; // 1/2
			}
		} else if (wr != 0) {
			// assume white sacrifices a rook against the lone pawn
			--w_maj;
			w_tot -= 2;
			if (false) {
			} else if (b_tot == 1) {
				ASSERT(b_maj == 0);
				ASSERT(b_min == 1);
				// KBK* or KNK*, always insufficient
				mul[Black] = 4; // 1/4
			} else if (b_tot == 2 && bn == 2) {
				ASSERT(b_maj == 0);
				ASSERT(b_min == 2);
				// KNNK*, usually insufficient
				mul[Black] = 4; // 1/4
			} else if (b_tot - w_tot <= 1 && b_maj <= 2) {
				// no more than 1 minor up, drawish
				mul[Black] = 8; // 1/2
			}
		}
	}

	// potential draw for white
	if (wt == wb + wp && wp >= 1) cflags[White] |= MatRookPawnFlag;
	if (wt == wb + wp && wb <= 1 && wp >= 1 && bt > bp) cflags[White] |= MatBishopFlag;

	if (wt == 2 && wn == 1 && wp == 1 && bt > bp) cflags[White] |= MatKnightFlag;

	// potential draw for black

	if (bt == bb + bp && bp >= 1) cflags[Black] |= MatRookPawnFlag;
	if (bt == bb + bp && bb <= 1 && bp >= 1 && wt > wp) cflags[Black] |= MatBishopFlag;

	if (bt == 2 && bn == 1 && bp == 1 && wt > wp) cflags[Black] |= MatKnightFlag;

	// king safety

	if (bq >= 1 && bq + br + bb + bn >= 2) cflags[White] |= MatKingFlag;
	if (wq >= 1 && wq + wr + wb + wn >= 2) cflags[Black] |= MatKingFlag;

	// phase (0: opening -> 256: endgame)
	S16 phase = TotalPhase;

	phase -= wp * PawnPhase;
	phase -= wn * KnightPhase;
	phase -= wb * BishopPhase;
	phase -= wr * RookPhase;
	phase -= wq * QueenPhase;

	phase -= bp * PawnPhase;
	phase -= bn * KnightPhase;
	phase -= bb * BishopPhase;
	phase -= br * RookPhase;
	phase -= bq * QueenPhase;

	if (phase < 0) phase = 0;

	ASSERT(phase >= 0 && phase <= TotalPhase);
	phase = (phase * 256 + (TotalPhase / 2)) / TotalPhase;

	ASSERT(phase >= 0 && phase <= 256);

	// material
	S16 opening = 0, endgame = 0;

	/* //TODO low material penalty
	 * // this one is asymmetrical - the side to move gets penalty if there are few pieces
	 * opening[me] += std::min((10 - wn+wb+2*(wr+wq)+bn+bb+2*(br+bq)),0)  *  PieceNumberBonus //10 is half of the maximum
	 * */
	/* Thomas */
	S32 owf, obf, ewf, ebf;
	owf = wn * KnightOpening + wb * BishopOpening + wr * RookOpening + wq * QueenOpening;
	//info->pv[White] = owf;
	opening += owf;
	opening += wp * PawnOpening;

	obf = bn * KnightOpening + bb * BishopOpening + br * RookOpening + bq * QueenOpening;
	//info->pv[Black] = obf;
	opening -= obf;
	opening -= bp * PawnOpening;

	ewf = wn * KnightEndgame + wb * BishopEndgame + wr * RookEndgame + wq * QueenEndgame;
	endgame += wp * PawnEndgame;
	endgame += ewf;

	ebf = bn * KnightEndgame + bb * BishopEndgame + br * RookEndgame + bq * QueenEndgame;
	endgame -= bp * PawnEndgame;
	endgame -= ebf;

	//S8 WhiteMinors,BlackMinors;
	S8 WhiteMajors, BlackMajors;
	//WhiteMinors = wn + wb;
	//BlackMinors = bn + bb;
	WhiteMajors = wq + wr;
	BlackMajors = bq + br;

	// Trade Bonus

	if (wm + WhiteMajors > bm + BlackMajors + 1) { // pieces over majors
		opening += bad_trade_value;
		endgame += bad_trade_value;
	} else if (bm + BlackMajors > wm + WhiteMajors + 1) {
		opening -= bad_trade_value;
		endgame -= bad_trade_value;
	} /*else if (WhiteMajors != BlackMajors) { // major over pawns
	if (WhiteMajors > BlackMajors && wm >= bm) {
		opening += bad_trade_value;
		endgame += bad_trade_value;
	} else if (BlackMajors > WhiteMajors && bm >= wm) {
		opening -= bad_trade_value;
		endgame -= bad_trade_value;
	}
   }*/


	// bishop pair

	if (wb >= 2) { // HACK: assumes different colours
		opening += BishopPairOpening;
		endgame += BishopPairEndgame;
	}

	if (bb >= 2) { // HACK: assumes different colours
		opening -= BishopPairOpening;
		endgame -= BishopPairEndgame;
	}

	// two knight penalty
	if (wn >= 2) {
		opening -= 10;
		endgame -= 10;
	}

	if (bn >= 2) {
		opening += 10;
		endgame += 10;
	}

	// two rook penalty
	if (wr >= 2) {
		opening -= 10;
		endgame -= 10;
	}

	if (br >= 2) {
		opening += 10;
		endgame += 10;
	}


	// rook score adjustment for number of pawns
	if (wr) {
		opening += RookPawnPenalty[wp + bp] * wr;
		endgame += RookPawnPenalty[wp + bp] * wr;
	}

	if (br) {
		opening -= RookPawnPenalty[wp + bp] * br;
		endgame -= RookPawnPenalty[wp + bp] * br;
	}

	// Queen and knight are better than queen and bishop.
	if (!wr && !br) {
		if (wq && wn) {
			opening += Queen_Knight_combo;
			endgame += Queen_Knight_combo;
		}
		if (bq && bn) {
			opening -= Queen_Knight_combo;
			endgame -= Queen_Knight_combo;
		}
	}

	// Rook and bishop are better than rook and knight.
	if (!wq && !bq) {
		if (wr && wb) {
			opening += Rook_Bishop_combo;
			endgame += Rook_Bishop_combo;
		}
		if (br && bb) {
			opening -= Rook_Bishop_combo;
			endgame -= Rook_Bishop_combo;
		}
	}

	/* //TODO: test
	constexpr S16 RookElephantiasisOpeningPenalty = -10;
	constexpr S16 RookElephantiasisEndgamePenalty = -50;
	constexpr S16 QueenElephantiasisOpeningPenalty = -20;
	constexpr S16 QueenElephantiasisEndgamePenalty = -100;
	/* Elephantiasis effect by Harm Geert Muller. Stronger pieces lose value in the presence of lower-valued pieces of
	 the opponent, because the latter can easily interdict their access to part of the board. * /
	if (bb > 0 || bn > 0) {
		opening += RookElephantiasisOpeningPenalty * wr; //not depending on bb, bn? bb, bn and br separately?
		endgame += RookElephantiasisEndgamePenalty * wr;
		opening += QueenElephantiasisOpeningPenalty * wq;
		endgame += QueenElephantiasisEndgamePenalty * wq;
	} else if (br > 0) {
		opening += QueenElephantiasisOpeningPenalty * wq;
		endgame += QueenElephantiasisEndgamePenalty * wq;
	}
	if (wb > 0 || wn > 0) {
		opening -= RookElephantiasisOpeningPenalty * br;
		endgame -= RookElephantiasisEndgamePenalty * br;
		opening -= QueenElephantiasisOpeningPenalty * bq;
		endgame -= QueenElephantiasisEndgamePenalty * bq;
	} else if (wr > 0) {
		opening -= QueenElephantiasisOpeningPenalty * bq;
		endgame -= QueenElephantiasisEndgamePenalty * bq;
	}
	*/

	if (wp + bp + wm + bm + WhiteMajors + BlackMajors <= bitbase_pieces) flags |= MatBitbaseFlag; //remove me?

	// store info

	info->recog = recog;
	info->flags = flags;
	info->cflags[0] = cflags[0];
	info->cflags[1] = cflags[1];
	info->mul[0] = mul[0];
	info->mul[1] = mul[1];
	info->phase = phase;
	info->opening = opening;
	info->endgame = endgame;
}

// end of material.cpp
