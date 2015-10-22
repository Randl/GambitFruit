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

constexpr S32 DrawNodeFlag = 1 << 0;
constexpr S32 DrawBishopFlag = 1 << 1;

constexpr S32 MatRookPawnFlag = 1 << 0;
constexpr S32 MatBishopFlag = 1 << 1;
constexpr S32 MatKnightFlag = 1 << 2;
constexpr S32 MatKingFlag = 1 << 3;
constexpr S32 MatBitbaseFlag = 1 << 4;

// variables
extern U8 bitbase_pieces;

// types

struct material_info_t {
    U32 lock;
    S16 phase;
    S16 opening;
    S16 endgame;
    U8 recog;
    U8 flags;
    std::array<U8, ColourNb> cflags;
    std::array<U8, ColourNb> mul;
    //S32 pv[ColourNb]; /* Material without pawn and king */
};

// functions

extern void material_init();

extern void material_alloc();
extern void material_clear();

extern void material_get_info(material_info_t *info, const board_t *board);

#endif // !defined MATERIAL_H

// end of material.h
