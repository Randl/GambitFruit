// search_full.h

#ifndef SEARCH_FULL_H
#define SEARCH_FULL_H

// includes

#include "board.h"
#include "util.h"

// functions

extern void search_full_init(list_t *list, board_t *board);
extern S32 search_full_root(list_t *list, board_t *board, S32 depth, S32 search_type);
extern S32 full_quiescence(board_t *board, S32 alpha, S32 beta, S32 depth, S32 height, mv_t pv[]);
#endif // !defined SEARCH_FULL_H

// end of search_full.h

