
// material.h

#ifndef MATERIAL_H
#define MATERIAL_H

// includes

#include <cstdint>
#include "board.h"
#include "colour.h"
#include "util.h"

// constants

enum mat_dummy_t {
	MAT_NONE,
	//MAT_KK,
	//MAT_KBK, MAT_KKB,
	//MAT_KNK, MAT_KKN,
	//MAT_KPK, MAT_KKP,
	//MAT_KQKQ, MAT_KQKP, MAT_KPKQ,
	//MAT_KRKR, MAT_KRKP, MAT_KPKR,
	//MAT_KBKB, MAT_KBKP, MAT_KPKB, MAT_KBPK, MAT_KKBP,
	//MAT_KNKN, MAT_KNKP, MAT_KPKN, MAT_KNPK, MAT_KKNP,
	MAT_KRPKR, MAT_KRKRP,
	MAT_KBPKB, MAT_KBKBP,
	MAT_NB
};

constexpr int_fast32_t DrawNodeFlag    = 1 << 0;
constexpr int_fast32_t DrawBishopFlag  = 1 << 1;

constexpr int_fast32_t MatRookPawnFlag = 1 << 0;
constexpr int_fast32_t MatBishopFlag   = 1 << 1;
constexpr int_fast32_t MatKnightFlag   = 1 << 2;
constexpr int_fast32_t MatKingFlag     = 1 << 3;
constexpr int_fast32_t MatBitbaseFlag  = 1 << 4;

// types

struct material_info_t {
	uint_fast8_t cflags[ColourNb];
	uint_fast8_t mul[ColourNb];
	uint_fast32_t lock;
	int_fast16_t phase;
	int_fast16_t opening;
	int_fast16_t endgame;
	uint_fast8_t recog;
	uint_fast8_t flags;
	//int_fast32_t pv[ColourNb]; /* Material without pawn and king */
};

// functions

extern void material_init     ();

extern void material_alloc    ();
extern void material_clear    ();

extern void material_get_info (material_info_t * info, const board_t * board);

#endif // !defined MATERIAL_H

// end of material.h
