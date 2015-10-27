// trans.h

#ifndef TRANS_H
#define TRANS_H

// includes

#include "util.h"

// types

typedef struct trans trans_t;

// variables

extern trans_t Trans[1];
extern bool trans_endgame;

// functions

extern bool trans_is_ok(const trans_t *trans);

extern void trans_init(trans_t *trans);

extern void trans_alloc(trans_t *trans);

extern void trans_free(trans_t *trans);

extern void trans_clear(trans_t *trans);

extern void trans_inc_date(trans_t *trans);

extern void trans_store(trans_t *trans, U64 key, U16 move, S32 depth, S32 min_value, S32 max_value);

extern bool
	trans_retrieve(trans_t *trans, U64 key, S32 *move, S32 *min_depth, S32 *max_depth, S32 *min_value, S32 *max_value);

extern void trans_stats(const trans_t *trans);

#endif // !defined TRANS_H

// end of trans.h

