
// sort.cpp

// includes

#include "attack.h"
#include "board.h"
#include "colour.h"
#include "list.h"
#include "move.h"
#include "move_check.h"
#include "move_evasion.h"
#include "move_gen.h"
#include "move_legal.h"
#include "piece.h"
#include "search.h"
#include "see.h"
#include "sort.h"
#include "util.h"
#include "value.h"

// constants

static const int_fast32_t KillerNb = 2;

static const int_fast32_t HistorySize = 12 * 64;
static const int_fast32_t HistoryMax = 16384;

static const int_fast32_t TransScore   = +32766;
static const int_fast32_t GoodScore    =  +4000;
static const int_fast32_t KillerScore  =     +4;
static const int_fast32_t HistoryScore = -24000;
static const int_fast32_t BadScore     = -28000;

static const int_fast32_t CODE_SIZE = 256;

// macros

#define HISTORY_INC(depth) ((depth)*(depth))

// types

enum gen_t {
   GEN_ERROR,
   GEN_LEGAL_EVASION,
   GEN_TRANS,
   GEN_GOOD_CAPTURE,
   GEN_BAD_CAPTURE,
   GEN_KILLER,
   GEN_QUIET,
   GEN_EVASION_QS,
   GEN_CAPTURE_QS,
   GEN_CHECK_QS,
   GEN_END
};

enum test_t {
   TEST_ERROR,
   TEST_NONE,
   TEST_LEGAL,
   TEST_TRANS_KILLER,
   TEST_GOOD_CAPTURE,
   TEST_BAD_CAPTURE,
   TEST_KILLER,
   TEST_QUIET,
   TEST_CAPTURE_QS,
   TEST_CHECK_QS
};

// variables

static int_fast32_t PosLegalEvasion;
static int_fast32_t PosSEE;

static int_fast32_t PosEvasionQS;
static int_fast32_t PosCheckQS;
static int_fast32_t PosCaptureQS;

static int_fast32_t Code[CODE_SIZE];

static uint_fast16_t Killer[HeightMax][KillerNb];

static fail_high_stats_t FailHighStats[HistorySize];
static uint_fast16_t History[HistorySize];
static uint_fast16_t HistHit[HistorySize];
static uint_fast16_t HistTot[HistorySize];

// prototypes

static void note_captures     (list_t * list, const board_t * board);
static void note_quiet_moves  (list_t * list, const board_t * board);
static void note_moves_simple (list_t * list, const board_t * board);
static void note_mvv_lva      (list_t * list, const board_t * board);

static int_fast32_t  move_value        (int_fast32_t move, const board_t * board, int_fast32_t height, int_fast32_t trans_killer);
static int_fast32_t  capture_value     (int_fast32_t move, const board_t * board);
static int_fast32_t  quiet_move_value  (int_fast32_t move, const board_t * board);
static int_fast32_t  move_value_simple (int_fast32_t move, const board_t * board);

static int_fast32_t  history_prob      (int_fast32_t move, const board_t * board);

static bool capture_is_good   (int_fast32_t move, const board_t * board);

static int_fast32_t  mvv_lva           (int_fast32_t move, const board_t * board);

static int_fast32_t  history_index     (int_fast32_t move, const board_t * board);

// functions

// sort_init()

void sort_init() {

   int_fast32_t i, height;
   int_fast32_t pos;

   // killer

   for (height = 0; height < HeightMax; height++) {
      for (i = 0; i < KillerNb; i++) Killer[height][i] = MoveNone;
   }

   // history

   for (i = 0; i < HistorySize; i++) History[i] = 0;

   for (i = 0; i < HistorySize; i++) {
      HistHit[i] = 1;
      HistTot[i] = 1;
	  FailHighStats[i].success = 1;
	  FailHighStats[i].tried = 1;
   }

   // Code[]

   for (pos = 0; pos < CODE_SIZE; pos++) Code[pos] = GEN_ERROR;

   pos = 0;

   // main search

   PosLegalEvasion = pos;
   Code[pos++] = GEN_LEGAL_EVASION;
   Code[pos++] = GEN_END;

   PosSEE = pos;
   Code[pos++] = GEN_TRANS;
   Code[pos++] = GEN_GOOD_CAPTURE;
   Code[pos++] = GEN_KILLER;
   Code[pos++] = GEN_QUIET;
   Code[pos++] = GEN_BAD_CAPTURE;
   Code[pos++] = GEN_END;

   // quiescence search

   PosEvasionQS = pos;
   Code[pos++] = GEN_EVASION_QS;
   Code[pos++] = GEN_END;

   PosCheckQS = pos;
   Code[pos++] = GEN_CAPTURE_QS;
   Code[pos++] = GEN_CHECK_QS;
   Code[pos++] = GEN_END;

   PosCaptureQS = pos;
   Code[pos++] = GEN_CAPTURE_QS;
   Code[pos++] = GEN_END;

   ASSERT(pos<CODE_SIZE);
}

// sort_init()

void sort_init(sort_t * sort, board_t * board, const attack_t * attack, int_fast32_t depth, int_fast32_t height, int_fast32_t trans_killer) {

   ASSERT(sort!=nullptr);
   ASSERT(board!=nullptr);
   ASSERT(attack!=nullptr);
   ASSERT(depth_is_ok(depth));
   ASSERT(height_is_ok(height));
   ASSERT(trans_killer==MoveNone||move_is_ok(trans_killer));

   sort->board = board;
   sort->attack = attack;

   sort->depth = depth;
   sort->height = height;

   sort->trans_killer = trans_killer;
   sort->killer_1 = Killer[sort->height][0];
   sort->killer_2 = Killer[sort->height][1];
   if (sort->height > 2){
	  sort->killer_3 = Killer[sort->height-2][0];
      sort->killer_4 = Killer[sort->height-2][1]; 
   }
   else{
      sort->killer_3 = MoveNone;
      sort->killer_4 = MoveNone;
   }

   if (ATTACK_IN_CHECK(sort->attack)) {

      gen_legal_evasions(sort->list,sort->board,sort->attack);
      note_moves(sort->list,sort->board,sort->height,sort->trans_killer);
      list_sort(sort->list);

      sort->gen = PosLegalEvasion + 1;
      sort->test = TEST_NONE;

   } else { // not in check

      LIST_CLEAR(sort->list);
      sort->gen = PosSEE;
   }

   sort->pos = 0;
}

// sort_next()

int_fast32_t sort_next(sort_t * sort) {

   int_fast32_t move;
   int_fast32_t gen;

   ASSERT(sort!=nullptr);

   while (true) {

      while (sort->pos < LIST_SIZE(sort->list)) {

         // next move

         move = LIST_MOVE(sort->list,sort->pos);
         sort->value = 16384; // default score
         sort->pos++;

         ASSERT(move!=MoveNone);

         // test

         if (false) {

         } else if (sort->test == TEST_NONE) {

            // no-op

         } else if (sort->test == TEST_TRANS_KILLER) {

            if (!move_is_pseudo(move,sort->board)) continue;
            if (!pseudo_is_legal(move,sort->board)) continue;

         } else if (sort->test == TEST_GOOD_CAPTURE) {

            ASSERT(move_is_tactical(move,sort->board));

            if (move == sort->trans_killer) continue;

            if (!capture_is_good(move,sort->board)) {
               LIST_ADD(sort->bad,move);
               continue;
            }

            if (!pseudo_is_legal(move,sort->board)) continue;

         } else if (sort->test == TEST_BAD_CAPTURE) {

            ASSERT(move_is_tactical(move,sort->board));
            ASSERT(!capture_is_good(move,sort->board));

            ASSERT(move!=sort->trans_killer);
            if (!pseudo_is_legal(move,sort->board)) continue;

         } else if (sort->test == TEST_KILLER) {

            if (move == sort->trans_killer) continue;
            if (!quiet_is_pseudo(move,sort->board)) continue;
            if (!pseudo_is_legal(move,sort->board)) continue;

            ASSERT(!move_is_tactical(move,sort->board));

         } else if (sort->test == TEST_QUIET) {

            ASSERT(!move_is_tactical(move,sort->board));

            if (move == sort->trans_killer) continue;
            if (move == sort->killer_1) continue;
            if (move == sort->killer_2) continue;
			if (move == sort->killer_3) continue;
            if (move == sort->killer_4) continue;
            if (!pseudo_is_legal(move,sort->board)) continue;

            sort->value = history_prob(move,sort->board);

         } else {

            ASSERT(false);

            return MoveNone;
         }

         ASSERT(pseudo_is_legal(move,sort->board));

         return move;
      }

      // next stage

      gen = Code[sort->gen++];

      if (false) {

      } else if (gen == GEN_TRANS) {

         LIST_CLEAR(sort->list);
         if (sort->trans_killer != MoveNone) LIST_ADD(sort->list,sort->trans_killer);

         sort->test = TEST_TRANS_KILLER;

      } else if (gen == GEN_GOOD_CAPTURE) {

         gen_captures(sort->list,sort->board);
         note_mvv_lva(sort->list,sort->board);
         list_sort(sort->list);

         LIST_CLEAR(sort->bad);

         sort->test = TEST_GOOD_CAPTURE;

      } else if (gen == GEN_BAD_CAPTURE) {

         list_copy(sort->list,sort->bad);

         sort->test = TEST_BAD_CAPTURE;

      } else if (gen == GEN_KILLER) {

         LIST_CLEAR(sort->list);
         if (sort->killer_1 != MoveNone) LIST_ADD(sort->list,sort->killer_1);
         if (sort->killer_2 != MoveNone) LIST_ADD(sort->list,sort->killer_2);
		 if (sort->killer_3 != MoveNone) LIST_ADD(sort->list,sort->killer_3);
         if (sort->killer_4 != MoveNone) LIST_ADD(sort->list,sort->killer_4);

         sort->test = TEST_KILLER;

      } else if (gen == GEN_QUIET) {

         gen_quiet_moves(sort->list,sort->board);
         note_quiet_moves(sort->list,sort->board);
         list_sort(sort->list);

         sort->test = TEST_QUIET;

      } else {

         ASSERT(gen==GEN_END);

         return MoveNone;
      }

      sort->pos = 0;
   }
}

// sort_init_qs()

void sort_init_qs(sort_t * sort, board_t * board, const attack_t * attack, bool check) {

   ASSERT(sort!=nullptr);
   ASSERT(board!=nullptr);
   ASSERT(attack!=nullptr);
   ASSERT(check==true||check==false);

   sort->board = board;
   sort->attack = attack;

   if (ATTACK_IN_CHECK(sort->attack)) {
      sort->gen = PosEvasionQS;
   } else if (check) {
      sort->gen = PosCheckQS;
   } else {
      sort->gen = PosCaptureQS;
   }

   LIST_CLEAR(sort->list);
   sort->pos = 0;
}

// sort_next_qs()

int_fast32_t sort_next_qs(sort_t * sort) {

   int_fast32_t move;
   int_fast32_t gen;

   ASSERT(sort!=nullptr);

   while (true) {

      while (sort->pos < LIST_SIZE(sort->list)) {

         // next move

         move = LIST_MOVE(sort->list,sort->pos);
         sort->pos++;

         ASSERT(move!=MoveNone);

         // test

         if (false) {

         } else if (sort->test == TEST_LEGAL) {

            if (!pseudo_is_legal(move,sort->board)) continue;

         } else if (sort->test == TEST_CAPTURE_QS) {

            ASSERT(move_is_tactical(move,sort->board));

            if (!capture_is_good(move,sort->board)) continue;
            if (!pseudo_is_legal(move,sort->board)) continue;

         } else if (sort->test == TEST_CHECK_QS) {

            ASSERT(!move_is_tactical(move,sort->board));
            ASSERT(move_is_check(move,sort->board));

            if (see_move(move,sort->board) < 0) continue;
            if (!pseudo_is_legal(move,sort->board)) continue;

         } else {

            ASSERT(false);

            return MoveNone;
         }

         ASSERT(pseudo_is_legal(move,sort->board));

         return move;
      }

      // next stage

      gen = Code[sort->gen++];

      if (false) {

      } else if (gen == GEN_EVASION_QS) {

         gen_pseudo_evasions(sort->list,sort->board,sort->attack);
         note_moves_simple(sort->list,sort->board);
         list_sort(sort->list);

         sort->test = TEST_LEGAL;

      } else if (gen == GEN_CAPTURE_QS) {

         gen_captures(sort->list,sort->board);
         note_mvv_lva(sort->list,sort->board);
         list_sort(sort->list);

         sort->test = TEST_CAPTURE_QS;

      } else if (gen == GEN_CHECK_QS) {

         gen_quiet_checks(sort->list,sort->board);

         sort->test = TEST_CHECK_QS;

      } else {

         ASSERT(gen==GEN_END);

         return MoveNone;
      }

      sort->pos = 0;
   }
}

// good_move()

void good_move(int_fast32_t move, const board_t * board, int_fast32_t depth, int_fast32_t height) {

   int_fast32_t index;
   int_fast32_t i;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);
   ASSERT(depth_is_ok(depth));
   ASSERT(height_is_ok(height));


   if (move_is_tactical(move,board)) return;

   // killer

   if (Killer[height][0] != move) {
      Killer[height][1] = Killer[height][0];
      Killer[height][0] = move;
   }

   ASSERT(Killer[height][0]==move);
   ASSERT(Killer[height][1]!=move);

   // history

   index = history_index(move,board);

   History[index] += HISTORY_INC(depth);

   if (History[index] >= HistoryMax) {
      for (i = 0; i < HistorySize; i++) {
         History[i] = (History[i] + 1) / 2;
      }
   } 
}

// history_good()

void history_good(int_fast32_t move, const board_t * board) {

   int_fast32_t index;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);

   if (move_is_tactical(move,board)) return;

   // history

   index = history_index(move,board);

   HistHit[index]++;
   HistTot[index]++;

   if (HistTot[index] >= HistoryMax) {
      HistHit[index] = (HistHit[index] + 1) / 2;
      HistTot[index] = (HistTot[index] + 1) / 2;
   }

   ASSERT(HistHit[index]<=HistTot[index]);
   ASSERT(HistTot[index]<HistoryMax);
}

// history_bad()

void history_bad(int_fast32_t move, const board_t * board) {

   int_fast32_t index;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);

   if (move_is_tactical(move,board)) return;

   // history

   index = history_index(move,board);

   HistTot[index]++;

   if (HistTot[index] >= HistoryMax) {
      HistHit[index] = (HistHit[index] + 1) / 2;
      HistTot[index] = (HistTot[index] + 1) / 2;
   }

   ASSERT(HistHit[index]<=HistTot[index]);
   ASSERT(HistTot[index]<HistoryMax);
}

// history_very_bad()

void history_very_bad(int_fast32_t move, const board_t * board) {

   int_fast32_t index;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);

   //if (move_is_tactical(move,board)) return;

   // history

   index = PIECE_TO_12(board->square[MOVE_TO(move)]) * 64 + SQUARE_TO_64(MOVE_TO(move));

   HistTot[index] += 100;

   if (HistTot[index] >= HistoryMax) {
      HistHit[index] = (HistHit[index] + 1) / 2;
      HistTot[index] = (HistTot[index] + 1) / 2;
   }

   ASSERT(HistHit[index]<=HistTot[index]);
   ASSERT(HistTot[index]<HistoryMax);
}

// history_tried()

void history_tried(int_fast32_t move, const board_t * board) {

   int_fast32_t index;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);

   if (move_is_tactical(move,board)) return;

   // history

   index = history_index(move,board);

   FailHighStats[index].tried++;
}

// history_success()

void history_success(int_fast32_t move, const board_t * board) {

   int_fast32_t index;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);

   if (move_is_tactical(move,board)) return;

   // history

   index = history_index(move,board);

   FailHighStats[index].success++;
}

bool history_reduction(int_fast32_t move, const board_t * board) {
	 
   int_fast32_t index;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);

   // history

   index = history_index(move,board);

   if(FailHighStats[index].success > FailHighStats[index].tried / 8) 
	   return false;
   return true;
}

// note_moves()

void note_moves(list_t * list, const board_t * board, int_fast32_t height, int_fast32_t trans_killer) {

   int_fast32_t size;
   int_fast32_t i, move;

   ASSERT(list_is_ok(list));
   ASSERT(board!=nullptr);
   ASSERT(height_is_ok(height));
   ASSERT(trans_killer==MoveNone||move_is_ok(trans_killer));

   size = LIST_SIZE(list);

   if (size >= 2) {
      for (i = 0; i < size; i++) {
         move = LIST_MOVE(list,i);
         list->value[i] = move_value(move,board,height,trans_killer);
      }
   }
}

// note_captures()

static void note_captures(list_t * list, const board_t * board) {

   int_fast32_t size;
   int_fast32_t i, move;

   ASSERT(list_is_ok(list));
   ASSERT(board!=nullptr);

   size = LIST_SIZE(list);

   if (size >= 2) {
      for (i = 0; i < size; i++) {
         move = LIST_MOVE(list,i);
         list->value[i] = capture_value(move,board);
      }
   }
}

// note_quiet_moves()

static void note_quiet_moves(list_t * list, const board_t * board) {

   int_fast32_t size;
   int_fast32_t i, move;

   ASSERT(list_is_ok(list));
   ASSERT(board!=nullptr);

   size = LIST_SIZE(list);

   if (size >= 2) {
      for (i = 0; i < size; i++) {
         move = LIST_MOVE(list,i);
         list->value[i] = quiet_move_value(move,board);
      }
   }
}

// note_moves_simple()

static void note_moves_simple(list_t * list, const board_t * board) {

   int_fast32_t size;
   int_fast32_t i, move;

   ASSERT(list_is_ok(list));
   ASSERT(board!=nullptr);

   size = LIST_SIZE(list);

   if (size >= 2) {
      for (i = 0; i < size; i++) {
         move = LIST_MOVE(list,i);
         list->value[i] = move_value_simple(move,board);
      }
   }
}

// note_mvv_lva()

static void note_mvv_lva(list_t * list, const board_t * board) {

   int_fast32_t size;
   int_fast32_t i, move;

   ASSERT(list_is_ok(list));
   ASSERT(board!=nullptr);

   size = LIST_SIZE(list);

   if (size >= 2) {
      for (i = 0; i < size; i++) {
         move = LIST_MOVE(list,i);
         list->value[i] = mvv_lva(move,board);
      }
   }
}

// move_value()

static int_fast32_t move_value(int_fast32_t move, const board_t * board, int_fast32_t height, int_fast32_t trans_killer) {

   int_fast32_t value;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);
   ASSERT(height_is_ok(height));
   ASSERT(trans_killer==MoveNone||move_is_ok(trans_killer));

   if (false) {
   } else if (move == trans_killer) { // transposition table killer
      value = TransScore;
   } else if (move_is_tactical(move,board)) { // capture or promote
      value = capture_value(move,board);
   } else if (move == Killer[height][0]) { // killer 1
      value = KillerScore;
   } else if (move == Killer[height][1]) { // killer 2
      value = KillerScore - 2;
   } else if (height > 2 && move == Killer[height-2][0]) { // killer 3
      value = KillerScore - 1;
   } else if (height > 2 && move == Killer[height-2][1]) { // killer 4
      value = KillerScore - 3;
   } else { // quiet move
      value = quiet_move_value(move,board);
   }

   return value;
}

// capture_value()

static int_fast32_t capture_value(int_fast32_t move, const board_t * board) {

   int_fast32_t value;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);

   ASSERT(move_is_tactical(move,board));

   value = mvv_lva(move,board);

   if (capture_is_good(move,board)) {
      value += GoodScore;
   } else {
      value += BadScore;
   }

   ASSERT(value>=-30000&&value<=+30000);

   return value;
}

// quiet_move_value()

static int_fast32_t quiet_move_value(int_fast32_t move, const board_t * board) {

   int_fast32_t value;
   int_fast32_t index;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);

   ASSERT(!move_is_tactical(move,board));

   index = history_index(move,board);

   value = HistoryScore + History[index];
   ASSERT(value>=HistoryScore&&value<=KillerScore-4);

   return value;
}

// move_value_simple()

static int_fast32_t move_value_simple(int_fast32_t move, const board_t * board) {

   int_fast32_t value;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);

   value = HistoryScore;
   if (move_is_tactical(move,board)) value = mvv_lva(move,board);

   return value;
}

// history_prob()

static int_fast32_t history_prob(int_fast32_t move, const board_t * board) {

   int_fast32_t value;
   int_fast32_t index;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);

   ASSERT(!move_is_tactical(move,board));

   index = history_index(move,board);

   ASSERT(HistHit[index]<=HistTot[index]);
   ASSERT(HistTot[index]<HistoryMax);

   value = (HistHit[index] * 16384) / HistTot[index];
   ASSERT(value>=0&&value<=16384);

   return value;
}

// capture_is_good()

static bool capture_is_good(int_fast32_t move, const board_t * board) {

   int_fast32_t piece, capture;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);

   ASSERT(move_is_tactical(move,board));

   // special cases

   if (MOVE_IS_EN_PASSANT(move)) return true;
   if (move_is_under_promote(move)) return false; // REMOVE ME?

   // captures and queen promotes

   capture = board->square[MOVE_TO(move)];

   if (capture != Empty) {

      // capture

      ASSERT(move_is_capture(move,board));

      if (MOVE_IS_PROMOTE(move)) return true; // promote-capture

      piece = board->square[MOVE_FROM(move)];
      if (VALUE_PIECE(capture) >= VALUE_PIECE(piece)) return true;
   }

   return see_move(move,board) >= 0;
}

// mvv_lva()

static int_fast32_t mvv_lva(int_fast32_t move, const board_t * board) {

   int_fast32_t piece, capture, promote;
   int_fast32_t value;

   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);

   ASSERT(move_is_tactical(move,board));

   if (MOVE_IS_EN_PASSANT(move)) { // en-passant capture

      value = 5; // PxP

   } else if ((capture = board->square[MOVE_TO(move)]) != Empty) { // normal capture

      piece = board->square[MOVE_FROM(move)];

      value = PIECE_ORDER(capture) * 6 - PIECE_ORDER(piece) + 5;
      ASSERT(value>=0&&value<30);

   } else { // promote

      ASSERT(MOVE_IS_PROMOTE(move));

      promote = move_promote(move);

      value = PIECE_ORDER(promote) - 5;
      ASSERT(value>=-4&&value<0);
   }

   ASSERT(value>=-4&&value<+30);

   return value;
}

// history_index()

static int_fast32_t history_index(int_fast32_t move, const board_t * board) {

   int_fast32_t index;
   
   ASSERT(move_is_ok(move));
   ASSERT(board!=nullptr);

   ASSERT(!move_is_tactical(move,board));

   index = PIECE_TO_12(board->square[MOVE_FROM(move)]) * 64 + SQUARE_TO_64(MOVE_TO(move));
   
   ASSERT(index>=0&&index<HistorySize);

   return index;
}

// end of sort.cpp

