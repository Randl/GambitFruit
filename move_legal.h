// move_legal.h

#ifndef MOVE_LEGAL_H
#define MOVE_LEGAL_H

// includes

#include <cstdint>
#include "board.h"
#include "list.h"
#include "util.h"

// functions

extern bool move_is_pseudo(uint_fast16_t move, board_t *board);
extern bool quiet_is_pseudo(uint_fast16_t move, board_t *board);

extern bool pseudo_is_legal(uint_fast16_t move, board_t *board);

#endif // !defined MOVE_LEGAL_H

// end of move_legal.h

