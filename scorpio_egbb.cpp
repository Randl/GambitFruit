//
// Created by Evgenii on 26.08.2015.
//

#include "value.h"
#include "scorpio_egbb.h"
#include "option.h"

#if defined (_WIN32) || defined(_WIN64)

#include <windows.h>

#define EGBB_NAME "egbbdll.dll"
#else
#define EGBB_NAME "egbbso.so"
#define HMODULE void*
#define LoadLibrary(x) dlopen(x,RTLD_LAZY)
#define GetProcAddress dlsym
#endif

#define ADD_PIECE(type)  {\
         egbb_piece[total_pieces] = type;\
         egbb_square[total_pieces] = from;\
        ++total_pieces;\
};

enum {
    LOAD_NONE, LOAD_4MEN, SMART_LOAD, LOAD_5MEN
};
static U8 egbb_ram_load = LOAD_4MEN;

constexpr S8 max_pieces = 32;


PPROBE_EGBB probe_egbb;
bool        egbb_is_loaded; //bool?
typedef void (*PLOAD_EGBB)(const char *path, S32 cache_size, S32 load_options);

void load_egbb_library() {
	HMODULE hmod;

	const char *main_path = option_get("Scorpio Bitbases Path");

	U32 egbb_cache_size = option_get_int("Scorpio Bitbases Cache Size") * 1024 * 1024;

	// RAM options
	const char *string = option_get_string("Load Scorpio Bitbases in RAM");

	if (false) {
	} else if (my_string_equal(string, "4-men")) {
		egbb_ram_load = LOAD_4MEN;
	} else if (my_string_equal(string, "Smart Load")) {
		egbb_ram_load = SMART_LOAD;
	} else if (my_string_equal(string, "None")) {
		egbb_ram_load = LOAD_NONE;
	} else if (my_string_equal(string, "5-men")) {
		egbb_ram_load = LOAD_5MEN;
	} else {
		ASSERT(false);
		egbb_ram_load = LOAD_4MEN;
	}

	char path[256];
	strcpy(path, main_path);
	strcat(path, EGBB_NAME);

	if (hmod)
		FreeLibrary(hmod);
	if (hmod = LoadLibrary(path)) {
		PLOAD_EGBB load_egbb = (PLOAD_EGBB) GetProcAddress(hmod, "load_egbb_xmen");
		probe_egbb = (PPROBE_EGBB) GetProcAddress(hmod, "probe_egbb_xmen");

		load_egbb(main_path, egbb_cache_size, egbb_ram_load);
		egbb_is_loaded = true;
		//printf("Bitbase loaded\n");
	} else {
		egbb_is_loaded = false;
		//printf("Bitbase not loaded\n");
	}
}

bool bitbase_probe(const board_t *board, S32 value) {

	const S8 player = board->turn;
	S8 total_pieces = 2;
	S32 egbb_piece[max_pieces], egbb_square[max_pieces];

	for (S32 i = 0; i < max_pieces; ++i) {
		egbb_piece[i]  = 0;
		egbb_square[i] = 0;
	}

	egbb_piece[0] = _WKING;
	egbb_piece[1] = _BKING;
	for (S8 from = 0; from < 64; ++from) {
		if (board->square[SQUARE_FROM_64(from)] == Empty) continue;

		switch (board->square[SQUARE_FROM_64(from)]) {
			case WP: ADD_PIECE(_WPAWN);
		        break;
			case BP: ADD_PIECE(_BPAWN);
		        break;
			case WN: ADD_PIECE(_WKNIGHT);
		        break;
			case BN: ADD_PIECE(_BKNIGHT);
		        break;
			case WB: ADD_PIECE(_WBISHOP);
		        break;
			case BB: ADD_PIECE(_BBISHOP);
		        break;
			case WR: ADD_PIECE(_WROOK);
		        break;
			case BR: ADD_PIECE(_BROOK);
		        break;
			case WQ: ADD_PIECE(_WQUEEN);
		        break;
			case BQ: ADD_PIECE(_BQUEEN);
		        break;
			case WK:
				egbb_square[0] = from;
		        break;
			case BK:
				egbb_square[1] = from;
		        break;
			default:
				break;
		}
	}

	const S32 score = probe_egbb(player, egbb_piece, egbb_square);

	if (score != _NOTFOUND) {
		if (score == 0)
			value = ValueDraw;
		else
			value = score;
		return true;
	}
	return false;
}