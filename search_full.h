
// search_full.h

#ifndef SEARCH_FULL_H
#define SEARCH_FULL_H

// includes

#include "board.h"
#include "util.h"


constexpr uint_fast8_t MaxThread = 32;
// functions

extern void search_full_init (list_t * list, board_t * board);
extern int_fast32_t  search_full_root (list_t * list, board_t * board, int_fast32_t depth, int_fast32_t search_type);

#endif // !defined SEARCH_FULL_H

// end of search_full.h

