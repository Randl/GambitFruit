// piece.h

#ifndef PIECE_H
#define PIECE_H

// includes

#include <array>
#include "colour.h"
#include "util.h"

// constants

constexpr U16 WhitePawnFlag = 1 << 2;
constexpr U16 BlackPawnFlag = 1 << 3;
constexpr U16 KnightFlag = 1 << 4;
constexpr U16 BishopFlag = 1 << 5;
constexpr U16 RookFlag = 1 << 6;
constexpr U16 KingFlag = 1 << 7;

constexpr U16 PawnFlags = WhitePawnFlag | BlackPawnFlag;
constexpr U16 QueenFlags = BishopFlag | RookFlag;

constexpr U16 PieceNone64 = 0;
constexpr U16 WhitePawn64 = WhitePawnFlag;
constexpr U16 BlackPawn64 = BlackPawnFlag;
constexpr U16 Knight64 = KnightFlag;
constexpr U16 Bishop64 = BishopFlag;
constexpr U16 Rook64 = RookFlag;
constexpr U16 Queen64 = QueenFlags;
constexpr U16 King64 = KingFlag;

constexpr U16 PieceNone256 = 0;
constexpr U16 WhitePawn256 = WhitePawn64 | WhiteFlag;
constexpr U16 BlackPawn256 = BlackPawn64 | BlackFlag;
constexpr U16 WhiteKnight256 = Knight64 | WhiteFlag;
constexpr U16 BlackKnight256 = Knight64 | BlackFlag;
constexpr U16 WhiteBishop256 = Bishop64 | WhiteFlag;
constexpr U16 BlackBishop256 = Bishop64 | BlackFlag;
constexpr U16 WhiteRook256 = Rook64 | WhiteFlag;
constexpr U16 BlackRook256 = Rook64 | BlackFlag;
constexpr U16 WhiteQueen256 = Queen64 | WhiteFlag;
constexpr U16 BlackQueen256 = Queen64 | BlackFlag;
constexpr U16 WhiteKing256 = King64 | WhiteFlag;
constexpr U16 BlackKing256 = King64 | BlackFlag;
constexpr U16 PieceNb = 256;

constexpr U8 WhitePawn12 = 0;
constexpr U8 BlackPawn12 = 1;
constexpr U8 WhiteKnight12 = 2;
constexpr U8 BlackKnight12 = 3;
constexpr U8 WhiteBishop12 = 4;
constexpr U8 BlackBishop12 = 5;
constexpr U8 WhiteRook12 = 6;
constexpr U8 BlackRook12 = 7;
constexpr U8 WhiteQueen12 = 8;
constexpr U8 BlackQueen12 = 9;
constexpr U8 WhiteKing12 = 10;
constexpr U8 BlackKing12 = 11;

// macros

#define PAWN_MAKE(colour)        (PawnMake[colour])
#define PAWN_OPP(pawn)           ((pawn)^(WhitePawn256^BlackPawn256))

#define PIECE_COLOUR(piece)      (static_cast<S8>(((piece)&3)-1))
#define PIECE_TYPE(piece)        ((piece)&~3)

#define PIECE_IS_PAWN(piece)     (((piece)&PawnFlags)!=0)
#define PIECE_IS_KNIGHT(piece)   (((piece)&KnightFlag)!=0)
#define PIECE_IS_BISHOP(piece)   (((piece)&QueenFlags)==BishopFlag)
#define PIECE_IS_ROOK(piece)     (((piece)&QueenFlags)==RookFlag)
#define PIECE_IS_QUEEN(piece)    (((piece)&QueenFlags)==QueenFlags)
#define PIECE_IS_KING(piece)     (((piece)&KingFlag)!=0)
#define PIECE_IS_SLIDER(piece)   (((piece)&QueenFlags)!=0)

#define PIECE_TO_12(piece)       (PieceTo12[piece])

#define PIECE_ORDER(piece)       (PieceOrder[piece])

#define PAWN_MOVE_INC(colour)    (PawnMoveInc[colour])
#define PIECE_INC(piece)         (PieceInc[piece])

// types

typedef S8 inc_t;

// "constants"

constexpr std::array<S32, ColourNb> PawnMake = {WhitePawn256, BlackPawn256};

constexpr std::array<S32, 12> PieceFrom12 =
	{WhitePawn256, BlackPawn256, WhiteKnight256, BlackKnight256, WhiteBishop256, BlackBishop256, WhiteRook256,
	 BlackRook256, WhiteQueen256, BlackQueen256, WhiteKing256, BlackKing256,};


constexpr std::array<inc_t, ColourNb> PawnMoveInc = {+16, -16,};

constexpr std::array<inc_t, 8 + 1> KnightInc = {-33, -31, -18, -14, +14, +18, +31, +33, 0};

constexpr std::array<inc_t, 4 + 1> BishopInc = {-17, -15, +15, +17, 0};

constexpr std::array<inc_t, 4 + 1> RookInc = {-16, -1, +1, +16, 0};

constexpr std::array<inc_t, 8 + 1> QueenInc = {-17, -16, -15, -1, +1, +15, +16, +17, 0};

constexpr std::array<inc_t, 8 + 1> KingInc = {-17, -16, -15, -1, +1, +15, +16, +17, 0};

// variables

extern std::array<S32, PieceNb> PieceTo12;
extern std::array<S32, PieceNb> PieceOrder;

extern std::array<const inc_t *, PieceNb> PieceInc;

// functions

extern void piece_init();

extern bool piece_is_ok(S32 piece);

extern S32 piece_from_12(S32 piece_12);

extern S32 piece_to_char(S32 piece);
extern S32 piece_from_char(S32 c);

#endif // !defined PIECE_H

// end of piece.h
