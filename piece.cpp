
// piece.cpp

// includes

#include <cstring>

#include "colour.h"
#include "piece.h"
#include "util.h"

// "constants"

constexpr int_fast32_t PawnMake[ColourNb] = { WhitePawn256, BlackPawn256 };

constexpr int_fast32_t PieceFrom12[12] = {
   WhitePawn256,   BlackPawn256,
   WhiteKnight256, BlackKnight256,
   WhiteBishop256, BlackBishop256,
   WhiteRook256,   BlackRook256,
   WhiteQueen256,  BlackQueen256,
   WhiteKing256,   BlackKing256,
};

static const char PieceString[12+1] = "PpNnBbRrQqKk";

constexpr inc_t PawnMoveInc[ColourNb] = {
   +16, -16,
};

constexpr inc_t KnightInc[8+1] = {
   -33, -31, -18, -14, +14, +18, +31, +33, 0
};

constexpr inc_t BishopInc[4+1] = {
   -17, -15, +15, +17, 0
};

constexpr inc_t RookInc[4+1] = {
   -16, -1, +1, +16, 0
};

constexpr inc_t QueenInc[8+1] = {
   -17, -16, -15, -1, +1, +15, +16, +17, 0
};

constexpr inc_t KingInc[8+1] = {
   -17, -16, -15, -1, +1, +15, +16, +17, 0
};

// variables

int_fast32_t PieceTo12[PieceNb];
int_fast32_t PieceOrder[PieceNb];

const inc_t * PieceInc[PieceNb];

// functions

// piece_init()

void piece_init() {

   int_fast32_t piece, piece_12;

   // PieceTo12[]

   for (piece = 0; piece < PieceNb; piece++) PieceTo12[piece] = -1;

   for (piece_12 = 0; piece_12 < 12; piece_12++) {
      PieceTo12[PieceFrom12[piece_12]] = piece_12;
   }

   // PieceOrder[]

   for (piece = 0; piece < PieceNb; piece++) PieceOrder[piece] = -1;

   for (piece_12 = 0; piece_12 < 12; piece_12++) {
      PieceOrder[PieceFrom12[piece_12]] = piece_12 >> 1;
   }

   // PieceInc[]

   for (piece = 0; piece < PieceNb; piece++) {
      PieceInc[piece] = nullptr;
   }

   PieceInc[WhiteKnight256] = KnightInc;
   PieceInc[WhiteBishop256] = BishopInc;
   PieceInc[WhiteRook256]   = RookInc;
   PieceInc[WhiteQueen256]  = QueenInc;
   PieceInc[WhiteKing256]   = KingInc;

   PieceInc[BlackKnight256] = KnightInc;
   PieceInc[BlackBishop256] = BishopInc;
   PieceInc[BlackRook256]   = RookInc;
   PieceInc[BlackQueen256]  = QueenInc;
   PieceInc[BlackKing256]   = KingInc;
}

// piece_is_ok()

bool piece_is_ok(int_fast32_t piece) {

   if (piece < 0 || piece >= PieceNb) return false;

   if (PieceTo12[piece] < 0) return false;

   return true;
}

// piece_from_12()

int_fast32_t piece_from_12(int_fast32_t piece_12) {

   ASSERT(piece_12>=0&&piece_12<12);

   return PieceFrom12[piece_12];
}

// piece_to_char()

int_fast32_t piece_to_char(int_fast32_t piece) {

   ASSERT(piece_is_ok(piece));

   return PieceString[PIECE_TO_12(piece)];
}

// piece_from_char()

int_fast32_t piece_from_char(int_fast32_t c) {

   const char *ptr;

   ptr = strchr(PieceString,c);
   if (ptr == nullptr) return PieceNone256;

   return piece_from_12(ptr-PieceString);
}

// end of piece.cpp

