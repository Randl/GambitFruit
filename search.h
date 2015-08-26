// search.h

#ifndef SEARCH_H
#define SEARCH_H

// includes

#include <csetjmp>

#include "board.h"
#include "list.h"
#include "move.h"
#include "util.h"

// constants

constexpr int_fast8_t MultiPVMax = 10;

constexpr int_fast16_t DepthMax  = 64;
constexpr int_fast16_t HeightMax = 256;

constexpr int_fast8_t SearchNormal = 0;
constexpr int_fast8_t SearchShort  = 1;

constexpr int_fast8_t SearchUnknown = 0;
constexpr int_fast8_t SearchUpper   = 1;
constexpr int_fast8_t SearchLower   = 2;
constexpr int_fast8_t SearchExact   = 3;

// types

struct search_multipv_t {
	double       time;
	int_fast64_t node_nb;
	char         pv_string[512];
	int_fast32_t mate;
    int_fast32_t depth; // TODO: int_fast16_t?
    int_fast32_t max_depth; // int_fast16_t?
	int_fast32_t value;
};

struct search_param_t {
    uint_fast16_t move;
    int_fast32_t  best_move; // uint_fast16_t
    int_fast32_t  threat_move;  // uint_fast16_t
	bool         reduced;
};

struct search_input_t {
	double       time_limit_1;
	double       time_limit_2;
	board_t      board[1];
	list_t       list[1];
	int_fast32_t depth_limit;
	int_fast32_t multipv;
	bool         time_is_limited;
	bool         infinite;
	bool         depth_is_limited;
};

struct search_info_t {
	double       last_time;
	jmp_buf      buf;
	int_fast32_t check_nb;
	int_fast32_t check_inc;
	bool         can_stop;
	bool         stop;

};

struct search_root_t {
	list_t       list[1];
	int_fast32_t depth;
	int_fast32_t move_pos;
	int_fast32_t move_nb;
	int_fast32_t last_value;
    uint_fast16_t move;
	bool         bad_1;
	bool         bad_2;
	bool         change;
	bool         easy;
	bool         flag;
};

struct search_best_t {
	mv_t         pv[HeightMax];
	int_fast32_t value;
	int_fast32_t flags;
	int_fast32_t depth;
    uint_fast16_t move;
};

struct search_current_t {
	double       time;
	double       speed;
	double       cpu;
	int_fast64_t node_nb;
	board_t      board[1];
	my_timer_t   timer[1];
	int_fast32_t max_depth;
	int_fast32_t max_extensions; // Thomas
	int_fast32_t multipv;
};

// variables

extern search_param_t   SearchStack[HeightMax]; // Thomas
extern search_input_t   SearchInput[1];
extern search_info_t    SearchInfo[1];
extern search_best_t    SearchBest[MultiPVMax];
extern search_root_t    SearchRoot[1];
extern search_current_t SearchCurrent[1];

// functions

extern bool depth_is_ok(int_fast32_t depth);
extern bool height_is_ok(int_fast32_t height);

extern void search_clear();
extern void search();

extern void search_update_best();
extern void search_update_root();
extern void search_update_current();

extern void search_check();

#endif // !defined SEARCH_H

// end of search.h
