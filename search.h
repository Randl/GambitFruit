
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

constexpr int_fast32_t MultiPVMax = 10;

constexpr int_fast32_t DepthMax = 64;
constexpr int_fast32_t HeightMax = 256;

constexpr int_fast32_t SearchNormal = 0;
constexpr int_fast32_t SearchShort  = 1;

constexpr int_fast32_t SearchUnknown = 0;
constexpr int_fast32_t SearchUpper   = 1;
constexpr int_fast32_t SearchLower   = 2;
constexpr int_fast32_t SearchExact   = 3;

// types

struct search_multipv_t {
   int_fast32_t mate;
   int_fast32_t depth;
   int_fast32_t max_depth;
   int_fast32_t value;
   double time;
   int_fast64_t node_nb;
   char pv_string[512];

};

struct search_param_t {
   int_fast32_t move;
   int_fast32_t best_move;
   int_fast32_t threat_move;
   bool reduced;
};

struct search_input_t {
   board_t board[1];
   list_t list[1];
   bool infinite;
   bool depth_is_limited;
   int_fast32_t depth_limit;
   int_fast32_t multipv;
   bool time_is_limited;
   double time_limit_1;
   double time_limit_2;
};

struct search_info_t {
   jmp_buf buf;
   bool can_stop;
   bool stop;
   int_fast32_t check_nb;
   int_fast32_t check_inc;
   double last_time;
};

struct search_root_t {
   list_t list[1];
   int_fast32_t depth;
   int_fast32_t move;
   int_fast32_t move_pos;
   int_fast32_t move_nb;
   int_fast32_t last_value;
   bool bad_1;
   bool bad_2;
   bool change;
   bool easy;
   bool flag;
};

struct search_best_t {
   int_fast32_t move;
   int_fast32_t value;
   int_fast32_t flags;
   int_fast32_t depth;
   mv_t pv[HeightMax];
};

struct search_current_t {
   board_t board[1];
   my_timer_t timer[1];
   int_fast32_t max_depth;
   int_fast32_t max_extensions; // Thomas
   int_fast32_t multipv;
   int_fast64_t node_nb;
   double time;
   double speed;
   double cpu;
};

// variables

extern search_param_t SearchStack[HeightMax]; // Thomas
extern search_input_t SearchInput[1];
extern search_info_t SearchInfo[1];
extern search_best_t SearchBest[MultiPVMax];
extern search_root_t SearchRoot[1];
extern search_current_t SearchCurrent[1];

// functions

extern bool depth_is_ok           (int_fast32_t depth);
extern bool height_is_ok          (int_fast32_t height);

extern void search_clear          ();
extern void search                ();

extern void search_update_best    ();
extern void search_update_root    ();
extern void search_update_current ();

extern void search_check          ();

#endif // !defined SEARCH_H

// end of search.h

