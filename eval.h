// eval.h

#ifndef EVAL_H
#define EVAL_H

// includes

#include "board.h"
#include "util.h"

// functions

extern void eval_init();

extern S32 eval(/*const*/ board_t *board, S32 alpha, bool do_le, bool in_check);

#endif // !defined EVAL_H

// end of eval.h

