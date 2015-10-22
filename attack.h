// attack.h

#ifndef ATTACK_H
#define ATTACK_H

// includes

#include <cstdint>
#include <array>
#include "board.h"
#include "util.h"
#include "vector.h"

// macros

#define IS_IN_CHECK(board, colour)         (is_attacked((board),KING_POS((board),(colour)),COLOUR_OPP((colour))))

#define DELTA_INC_LINE(delta)             (DeltaIncLine[DeltaOffset+(delta)])
#define DELTA_INC_ALL(delta)              (DeltaIncAll[DeltaOffset+(delta)])
#define DELTA_MASK(delta)                 (DeltaMask[DeltaOffset+(delta)])

#define INC_MASK(inc)                     (IncMask[IncOffset+(inc)])

#define PIECE_ATTACK(board, piece, from, to) (PSEUDO_ATTACK((piece),(to)-(from))&&line_is_empty((board),(from),(to)))
#define PSEUDO_ATTACK(piece, delta)        (((piece)&DELTA_MASK(delta))!=0)
#define SLIDER_ATTACK(piece, inc)          (((piece)&INC_MASK(inc))!=0)

#define ATTACK_IN_CHECK(attack)           ((attack)->dn!=0)

// types

struct attack_t {
    std::array<S32, 2 + 1> ds;
    std::array<S32, 2 + 1> di;
    S32 dn;
};

// variables

extern std::array<S32, DeltaNb> DeltaIncLine;
extern std::array<S32, DeltaNb> DeltaIncAll;

extern std::array<S32, DeltaNb> DeltaMask;
extern std::array<S32, IncNb> IncMask;

// functions

extern void attack_init();

extern bool is_attacked(const board_t *board, S32 to, S8 colour);

extern bool line_is_empty(const board_t *board, S32 from, S32 to);

extern bool is_pinned(const board_t *board, S32 square, S8 colour);

extern bool attack_is_ok(const attack_t *attack);
extern void attack_set(attack_t *attack, const board_t *board);

extern bool piece_attack_king(const board_t *board, S32 piece, S32 from, S32 king);

#endif // !defined ATTACK_H

// end of attack.h
