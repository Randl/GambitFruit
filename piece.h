
// piece.h

#ifndef PIECE_H
#define PIECE_H

// includes

#include "colour.h"
#include "util.h"

// constants

const int_fast32_t WhitePawnFlag = 1 << 2;
const int_fast32_t BlackPawnFlag = 1 << 3;
const int_fast32_t KnightFlag    = 1 << 4;
const int_fast32_t BishopFlag    = 1 << 5;
const int_fast32_t RookFlag      = 1 << 6;
const int_fast32_t KingFlag      = 1 << 7;

const int_fast32_t PawnFlags  = WhitePawnFlag | BlackPawnFlag;
const int_fast32_t QueenFlags = BishopFlag | RookFlag;

const int_fast32_t PieceNone64 = 0;
const int_fast32_t WhitePawn64 = WhitePawnFlag;
const int_fast32_t BlackPawn64 = BlackPawnFlag;
const int_fast32_t Knight64    = KnightFlag;
const int_fast32_t Bishop64    = BishopFlag;
const int_fast32_t Rook64      = RookFlag;
const int_fast32_t Queen64     = QueenFlags;
const int_fast32_t King64      = KingFlag;

const int_fast32_t PieceNone256   = 0;
const int_fast32_t WhitePawn256   = WhitePawn64 | WhiteFlag;
const int_fast32_t BlackPawn256   = BlackPawn64 | BlackFlag;
const int_fast32_t WhiteKnight256 = Knight64    | WhiteFlag;
const int_fast32_t BlackKnight256 = Knight64    | BlackFlag;
const int_fast32_t WhiteBishop256 = Bishop64    | WhiteFlag;
const int_fast32_t BlackBishop256 = Bishop64    | BlackFlag;
const int_fast32_t WhiteRook256   = Rook64      | WhiteFlag;
const int_fast32_t BlackRook256   = Rook64      | BlackFlag;
const int_fast32_t WhiteQueen256  = Queen64     | WhiteFlag;
const int_fast32_t BlackQueen256  = Queen64     | BlackFlag;
const int_fast32_t WhiteKing256   = King64      | WhiteFlag;
const int_fast32_t BlackKing256   = King64      | BlackFlag;
const int_fast32_t PieceNb        = 256;

const int_fast32_t WhitePawn12   =  0;
const int_fast32_t BlackPawn12   =  1;
const int_fast32_t WhiteKnight12 =  2;
const int_fast32_t BlackKnight12 =  3;
const int_fast32_t WhiteBishop12 =  4;
const int_fast32_t BlackBishop12 =  5;
const int_fast32_t WhiteRook12   =  6;
const int_fast32_t BlackRook12   =  7;
const int_fast32_t WhiteQueen12  =  8;
const int_fast32_t BlackQueen12  =  9;
const int_fast32_t WhiteKing12   = 10;
const int_fast32_t BlackKing12   = 11;

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

// "constants"

extern const int_fast32_t PawnMake[ColourNb];
extern const int_fast32_t PieceFrom12[12];

extern const inc_t PawnMoveInc[ColourNb];

extern const inc_t KnightInc[8+1];
extern const inc_t BishopInc[4+1];
extern const inc_t RookInc[4+1];
extern const inc_t QueenInc[8+1];
extern const inc_t KingInc[8+1];

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

