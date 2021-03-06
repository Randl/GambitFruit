// square.h

#ifndef SQUARE_H
#define SQUARE_H

// includes

#include <array>
#include "colour.h"
#include "util.h"

// constants

constexpr U8 FileNb = 16;
constexpr U8 RankNb = 16;

constexpr U16 SquareNb = FileNb * RankNb;

constexpr U8 FileInc = +1;
constexpr U8 RankInc = +16;

constexpr U8 FileNone = 0;

constexpr U8 FileA = 0x4;
constexpr U8 FileB = 0x5;
constexpr U8 FileC = 0x6;
constexpr U8 FileD = 0x7;
constexpr U8 FileE = 0x8;
constexpr U8 FileF = 0x9;
constexpr U8 FileG = 0xA;
constexpr U8 FileH = 0xB;

constexpr U8 RankNone = 0;

constexpr U8 Rank1 = 0x4;
constexpr U8 Rank2 = 0x5;
constexpr U8 Rank3 = 0x6;
constexpr U8 Rank4 = 0x7;
constexpr U8 Rank5 = 0x8;
constexpr U8 Rank6 = 0x9;
constexpr U8 Rank7 = 0xA;
constexpr U8 Rank8 = 0xB;

constexpr U8 SquareNone = 0;

constexpr U16 A1 = 0x44, B1 = 0x45, C1 = 0x46, D1 = 0x47, E1 = 0x48, F1 = 0x49, G1 = 0x4A, H1 = 0x4B;
constexpr U16 A2 = 0x54, B2 = 0x55, C2 = 0x56, D2 = 0x57, E2 = 0x58, F2 = 0x59, G2 = 0x5A, H2 = 0x5B;
constexpr U16 A3 = 0x64, B3 = 0x65, C3 = 0x66, D3 = 0x67, E3 = 0x68, F3 = 0x69, G3 = 0x6A, H3 = 0x6B;
constexpr U16 A4 = 0x74, B4 = 0x75, C4 = 0x76, D4 = 0x77, E4 = 0x78, F4 = 0x79, G4 = 0x7A, H4 = 0x7B;
constexpr U16 A5 = 0x84, B5 = 0x85, C5 = 0x86, D5 = 0x87, E5 = 0x88, F5 = 0x89, G5 = 0x8A, H5 = 0x8B;
constexpr U16 A6 = 0x94, B6 = 0x95, C6 = 0x96, D6 = 0x97, E6 = 0x98, F6 = 0x99, G6 = 0x9A, H6 = 0x9B;
constexpr U16 A7 = 0xA4, B7 = 0xA5, C7 = 0xA6, D7 = 0xA7, E7 = 0xA8, F7 = 0xA9, G7 = 0xAA, H7 = 0xAB;
constexpr U16 A8 = 0xB4, B8 = 0xB5, C8 = 0xB6, D8 = 0xB7, E8 = 0xB8, F8 = 0xB9, G8 = 0xBA, H8 = 0xBB;

constexpr U8 Dark = 0;
constexpr U8 Light = 1;

// macros

#define SQUARE_IS_OK(square)        ((((square)-0x44)&~0x77)==0)

#define SQUARE_MAKE(file, rank)      (((rank)<<4)|(file))

#define SQUARE_FILE(square)         ((U8)((square)&0xF))
#define SQUARE_RANK(square)         ((square)>>4)

#define SQUARE_FROM_64(square)      (SquareFrom64[square])
#define SQUARE_TO_64(square)        (SquareTo64[square])

#define SQUARE_IS_PROMOTE(square)   (SquareIsPromote[square])
#define SQUARE_EP_DUAL(square)      ((square)^16)

#define SQUARE_COLOUR(square)       (((square)^((square)>>4))&1)

#define SQUARE_FILE_MIRROR(square)  ((square)^0x0F)
#define SQUARE_RANK_MIRROR(square)  ((square)^0xF0)

#define FILE_OPP(file)              ((file)^0xF)
#define RANK_OPP(rank)              ((rank)^0xF)

#define PAWN_RANK(square, colour)    (SQUARE_RANK(square)^RankMask[colour])
#define PAWN_PROMOTE(square, colour) (PromoteRank[colour]|((square)&0xF))

// types

typedef U16 sq_t; //U8

// "constants"


constexpr std::array<S32, 64> SquareFrom64 =
	{A1, B1, C1, D1, E1, F1, G1, H1, A2, B2, C2, D2, E2, F2, G2, H2, A3, B3, C3, D3, E3, F3, G3, H3, A4, B4, C4, D4, E4,
	 F4, G4, H4, A5, B5, C5, D5, E5, F5, G5, H5, A6, B6, C6, D6, E6, F6, G6, H6, A7, B7, C7, D7, E7, F7, G7, H7, A8, B8,
	 C8, D8, E8, F8, G8, H8,};

constexpr std::array<S16, ColourNb> RankMask = {0, 0xF};
constexpr std::array<S16, ColourNb> PromoteRank = {0xB0, 0x40};
// variables

extern std::array<S32, SquareNb> SquareTo64;
extern std::array<bool, SquareNb> SquareIsPromote;

// functions

extern void square_init();

extern S32 file_from_char(S32 c);
extern S32 rank_from_char(S32 c);

extern char file_to_char(S32 file);
extern char rank_to_char(S32 rank);

extern bool square_to_string(S32 square, char string[], S32 size);
extern S32 square_from_string(const char string[]);

#endif // !defined SQUARE_H

// end of square.h
