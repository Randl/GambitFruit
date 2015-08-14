
// random.cpp

// includes

#include "random.h"
#include "util.h"

// functions

// random_init()

void random_init() {
	if ((Random64[RandomNb-1] >> 32) != 0xF8D626AA) { // upper half of the last element of the array
		my_fatal("broken 64-bit types\n");
	}
}

// end of random.cpp