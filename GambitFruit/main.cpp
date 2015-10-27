// main.cpp

// includes

#include <cstdio>
#include <cstdlib>

#include "attack.h"
#include "book.h"
#include "hash.h"
#include "move_do.h"
#include "option.h"
#include "pawn.h"
#include "protocol.h"
#include "random.h"
#include "trans.h"
#include "value.h"

// main()

int main(int argc, char *argv[]) {

	// init

	util_init();
	my_random_init(); // for opening book

	printf(
		"Gambit Fruit based on Fruit 2.1 and Toga by Ryan Benitez, Thomas Gaksch and Fabien Letouzey\nEdit by Evgeniy Zheltonozhskiy\n");

	// early initialisation (the rest is done after UCI options are parsed in protocol.cpp)

	option_init();

	square_init();
	piece_init();
	pawn_init_bit();
	value_init();
	vector_init();
	attack_init();
	move_do_init();

	random_init();
	hash_init();

	trans_init(Trans);
	book_init();

	// loop
	loop();

	return EXIT_SUCCESS;
}


// end of main.cpp
