// eval.h

#ifndef EVAL_H
#define EVAL_H

// includes

#include "board.h"
#include "util.h"

// functions

extern void eval_init();

extern int_fast32_t eval(/*const*/ board_t *board, int_fast32_t alpha, int_fast32_t beta, bool do_le, bool in_check);

#endif // !defined EVAL_H

// end of eval.h

