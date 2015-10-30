// vector.cpp

// includes

#include "piece.h"
#include "vector.h"

// variables

std::array<S32, DeltaNb> Distance;

// functions

void vector_init() {

	// Distance[]
	for (S16 delta = 0; delta < DeltaNb; ++delta)
		Distance[delta] = -1;
	for (S8 y = -7; y <= +7; ++y)
		for (S8 x = -7; x <= +7; ++x) {

			const S32 delta = y * 16 + x;
			ASSERT(delta_is_ok(delta));

			S32 dist = 0, tmp = x;

			if (tmp < 0) tmp = -tmp;
			if (tmp > dist) dist = tmp;

			tmp = y;
			if (tmp < 0) tmp = -tmp;
			if (tmp > dist) dist = tmp;

			Distance[DeltaOffset + delta] = dist;
		}
}

// delta_is_ok()

bool delta_is_ok(S32 delta) {

	if (delta < -119 || delta > +119) return false;
	if ((delta & 0xF) == 8) return false; // HACK: delta % 16 would be ill-defined for negative numbers

	return true;
}

// inc_is_ok()

bool inc_is_ok(S32 inc) {

	S32 dir;

	for (dir = 0; dir < 8; ++dir)
		if (KingInc[dir] == inc) return true;

	return false;
}

// end of vector.cpp
