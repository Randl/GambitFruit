
// piece.h

#ifndef PIECE_H
#define PIECE_H

// includes

#include "colour.h"
#include "util.h"

// constexprants

constexprexpr int_fast32_t WhitePawnFlag = 1 << 2;
constexpr int_fast32_t BlackPawnFlag = 1 << 3;
constexpr int_fast32_t KnightFlag    = 1 << 4;
constexpr int_fast32_t BishopFlag    = 1 << 5;
constexpr int_fast32_t RookFlag      = 1 << 6;
constexpr int_fast32_t KingFlag      = 1 << 7;

constexpr int_fast32_t PawnFlags  = WhitePawnFlag | BlackPawnFlag;
constexpr int_fast32_t QueenFlags = BishopFlag | RookFlag;

constexpr int_fast32_t PieceNone64 = 0;
constexpr int_fast32_t WhitePawn64 = WhitePawnFlag;
constexpr int_fast32_t BlackPawn64 = BlackPawnFlag;
constexpr int_fast32_t Knight64    = KnightFlag;
constexpr int_fast32_t Bishop64    = BishopFlag;
constexpr int_fast32_t Rook64      = RookFlag;
constexpr int_fast32_t Queen64     = QueenFlags;
constexpr int_fast32_t King64      = KingFlag;

constexpr int_fast32_t PieceNone256   = 0;
constexpr int_fast32_t WhitePawn256   = WhitePawn64 | WhiteFlag;
constexpr int_fast32_t BlackPawn256   = BlackPawn64 | BlackFlag;
constexpr int_fast32_t WhiteKnight256 = Knight64    | WhiteFlag;
constexpr int_fast32_t BlackKnight256 = Knight64    | BlackFlag;
constexpr int_fast32_t WhiteBishop256 = Bishop64    | WhiteFlag;
constexpr int_fast32_t BlackBishop256 = Bishop64    | BlackFlag;
constexpr int_fast32_t WhiteRook256   = Rook64      | WhiteFlag;
constexpr int_fast32_t BlackRook256   = Rook64      | BlackFlag;
constexpr int_fast32_t WhiteQueen256  = Queen64     | WhiteFlag;
constexpr int_fast32_t BlackQueen256  = Queen64     | BlackFlag;
constexpr int_fast32_t WhiteKing256   = King64      | WhiteFlag;
constexpr int_fast32_t BlackKing256   = King64      | BlackFlag;
constexpr int_fast32_t PieceNb        = 256;

constexpr int_fast32_t WhitePawn12   =  0;
constexpr int_fast32_t BlackPawn12   =  1;
constexpr int_fast32_t WhiteKnight12 =  2;
constexpr int_fast32_t BlackKnight12 =  3;
constexpr int_fast32_t WhiteBishop12 =  4;
constexpr int_fast32_t BlackBishop12 =  5;
constexpr int_fast32_t WhiteRook12   =  6;
constexpr int_fast32_t BlackRook12   =  7;
constexpr int_fast32_t WhiteQueen12  =  8;
constexpr int_fast32_t BlackQueen12  =  9;
constexpr int_fast32_t WhiteKing12   = 10;
constexpr int_fast32_t BlackKing12   = 11;

// macros

#define PAWN_MAKE(colour)        (PawnMake[colour])
#define PAWN_OPP(pawn)           ((pawn)^(WhitePawn256^BlackPawn256))

#define PIECE_COLOUR(piece)      (((piece)&3)-1)
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

typedef int_fast32_t inc_t;

// "constexprants"

extern constexpr int_fast32_t PawnMake[ColourNb];
extern constexpr int_fast32_t PieceFrom12[12];

extern constexpr inc_t PawnMoveInc[ColourNb];

extern constexpr inc_t KnightInc[8+1];
extern constexpr inc_t BishopInc[4+1];
extern constexpr inc_t RookInc[4+1];
extern constexpr inc_t QueenInc[8+1];
extern constexpr inc_t KingInc[8+1];

// variables

extern int_fast32_t PieceTo12[PieceNb];
extern int_fast32_t PieceOrder[PieceNb];

extern const inc_t * PieceInc[PieceNb];

// functions

extern void piece_init      ();

extern bool piece_is_ok     (int_fast32_t piece);

extern int_fast32_t  piece_from_12   (int_fast32_t piece_12);

extern int_fast32_t  piece_to_char   (int_fast32_t piece);
extern int_fast32_t  piece_from_char (int_fast32_t c);

#endif // !defined PIECE_H

// end of piece.h

