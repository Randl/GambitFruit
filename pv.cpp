
// pv.cpp

// includes

#include <cstring>

#include "board.h"
#include "move.h"
#include "move_do.h"
#include "pv.h"
#include "util.h"

// functions

// pv_is_ok()

bool pv_is_ok(const mv_t pv[]) {

	if (pv == nullptr) return false;

	for (int_fast32_t pos = 0; true; ++pos) {
		if (pos >= 256) return false;
		int_fast32_t move = pv[pos];

		if (move == MoveNone) return true;
		if (!move_is_ok(move)) return false;
	}

	return true;
}

// pv_copy()

void pv_copy(mv_t dst[], const mv_t src[]) {

	ASSERT(pv_is_ok(src));
	ASSERT(dst!=nullptr);

	while ((*dst++ = *src++) != MoveNone);
}

// pv_cat()

void pv_cat(mv_t dst[], const mv_t src[], int_fast32_t move) {

	ASSERT(pv_is_ok(src));
	ASSERT(dst!=nullptr);

	*dst++ = move;
	while ((*dst++ = *src++) != MoveNone);
}

// pv_to_string()

bool pv_to_string(const mv_t pv[], char string[], int_fast32_t size) {

	ASSERT(pv_is_ok(pv));
	ASSERT(string!=nullptr);
	ASSERT(size>=512);

	// init
	if (size < 512) return false;
	int_fast32_t pos = 0;

	// loop
	int_fast32_t move;
	while ((move = *pv++) != MoveNone) {
		if (pos != 0) string[pos++] = ' ';

		move_to_string(move,&string[pos],size-pos);
		pos += strlen(&string[pos]);
	}

	string[pos] = '\0';
	return true;
}

// end of pv.cpp
