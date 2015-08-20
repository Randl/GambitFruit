// search_full.cpp

// includes

#include <cstdlib>
#include "attack.h"
#include "eval.h"
#include "list.h"
#include "move.h"
#include "move_check.h"
#include "move_do.h"
#include "option.h"
#include "pv.h"
#include "search.h"
#include "search_full.h"
#include "see.h"
#include "sort.h"
#include "trans.h"
#include "value.h"

// constants and variables
int_fast32_t ValueDraw;


// main search

// transposition table

static constexpr int_fast8_t TransDepth = 1;

// null move

static /* const */ bool      UseNull     = true;
static /* const */ bool      UseNullEval = true; // true
static constexpr int_fast8_t NullDepth   = 2;
static /* const */ int_fast16_t NullReduction = 3;

static /* const */ bool         UseVer        = true;
static /* const */ bool         UseVerEndgame = true; // true
static /* const */ int_fast16_t VerReduction  = 5; // was 3

// move ordering

static constexpr int_fast8_t IIDDepth = 3;
static constexpr int_fast8_t IIDReduction = 2;

// extensions

// rebel reduction
static bool use_rebel_reduction = false; // I hope I did this right...

// razoring

static constexpr int_fast8_t RazorDepth = 3;
static constexpr int_fast16_t RazorMargin = 300;

// history pruning

static /* const */ bool         UseHistory         = true;
static constexpr int_fast8_t    HistoryDepth       = 3; // was 3
static constexpr int_fast8_t    HistoryMoveNb      = 3; // was 3
static /* const */ int_fast32_t HistoryValue       = 9830; // 60%
static /* const */ int_fast16_t HistoryBound       = 2458; // * 16384 + 50) / 100 10%=1638 15%=2458 20%=3277
static /* const */ bool         UseExtendedHistory = true;
static bool                     research_on_beta   = true;

// futility pruning

static /* const */ bool         UseFutility          = true; // false
static constexpr int_fast16_t   FutilityMargin       = 100;
//static bool quick_futility = false;
static /* const */ int_fast16_t FutilityMargin1      = 100;
static /* const */ int_fast16_t FutilityMargin2      = 200;
static /* const */ int_fast16_t FutilityMargin3      = 350;
static /* const */ int_fast8_t  FutilityPruningDepth = 3; // was 3
// quiescence search

static /* const */ bool UseDelta = true; // false
static /* const */ int_fast16_t DeltaMargin = 50;

static /* const */ int_fast8_t CheckNb = 1;
static /* const */ int_fast8_t CheckDepth = 0; // 1 - CheckNb

// misc

static constexpr int_fast8_t NodeAll = -1;
static constexpr int_fast8_t    NodePV               = 0;
static constexpr int_fast8_t NodeCut = +1;

// macros

#define NODE_OPP(type)     (-(type))
#define DEPTH_MATCH(d1, d2) ((d1)>=(d2))

// prototypes

static int_fast32_t full_root(list_t *list, board_t *board, int_fast32_t alpha, int_fast32_t beta, int_fast32_t depth,
							  int_fast32_t height, int_fast32_t search_type);

static int_fast32_t full_search(board_t *board, int_fast32_t alpha, int_fast32_t beta, int_fast32_t depth,
								int_fast32_t height, mv_t pv[], int_fast32_t node_type);
static int_fast32_t full_no_null(board_t *board, int_fast32_t alpha, int_fast32_t beta, int_fast32_t depth,
								 int_fast32_t height, mv_t pv[], int_fast32_t node_type, int_fast32_t trans_move,
								 int_fast32_t *best_move);

static int_fast32_t full_quiescence(board_t *board, int_fast32_t alpha, int_fast32_t beta, int_fast32_t depth,
									int_fast32_t height, mv_t pv[]);

static int_fast32_t full_new_depth(int_fast32_t depth, int_fast32_t move, board_t *board, bool single_reply, bool in_pv,
								   int_fast32_t height);

static bool do_null(const board_t *board);
static bool do_ver(const board_t *board);

static void pv_fill(const mv_t pv[], board_t *board);

static bool move_is_dangerous(int_fast32_t move, const board_t *board);
static bool capture_is_dangerous(int_fast32_t move, const board_t *board);

static bool simple_stalemate(const board_t *board);

// functions

// search_full_init()

void search_full_init(list_t *list, board_t *board) {

	ASSERT(list->is_ok());
	ASSERT(board_is_ok(board));

	// draw value
	ValueDraw = option_get_int("Contempt Factor");

	// null-move options
	const char *string = option_get_string("Null Move Pruning");

	if (false) {
	} else if (my_string_equal(string, "Always")) {
		UseNull = true;
		UseNullEval = false;
	} else if (my_string_equal(string, "Fail High")) {
		UseNull = true;
		UseNullEval = true;
	} else if (my_string_equal(string, "Never")) {
		UseNull = false;
		UseNullEval = false;
	} else {
		ASSERT(false);
		UseNull = true;
		UseNullEval = true;
	}

	NullReduction = option_get_int("Null Move Reduction");

	string = option_get_string("Verification Search");

	if (false) {
	} else if (my_string_equal(string, "Always")) {
		UseVer = true;
		UseVerEndgame = false;
	} else if (my_string_equal(string, "Endgame")) {
		UseVer = true;
		UseVerEndgame = true;
	} else if (my_string_equal(string, "Never")) {
		UseVer = false;
		UseVerEndgame = false;
	} else {
		ASSERT(false);
		UseVer = true;
		UseVerEndgame = true;
	}

	VerReduction = option_get_int("Verification Reduction");

	// rebel reduction options
	use_rebel_reduction = option_get_bool("Rebel Reductions");

	// history-pruning options
	UseHistory = option_get_bool("History Pruning");
	HistoryValue = (option_get_int("History Threshold") * 16384 + 50) / 100;
	research_on_beta = option_get_bool("History Research on Beta");

	// futility-pruning options
	UseFutility = option_get_bool("Futility Pruning");

	//quick_futility = option_get_bool("Quick Futility eval");
	FutilityMargin1 = option_get_int("Futility Margin");
	FutilityMargin3 = option_get_int("Extended Futility Margin");
	FutilityPruningDepth = option_get_int("Futility Pruning Depth");

	// delta-pruning options
	UseDelta = option_get_bool("Delta Pruning");
	DeltaMargin = option_get_int("Delta Margin");

	// quiescence-search options
	CheckNb = option_get_int("Quiescence Check Plies");
	CheckDepth = 1 - CheckNb;

	// standard sort
	list->note();
	list->sort();

	// basic sort
	int_fast32_t trans_move, trans_min_depth, trans_max_depth, trans_min_value, trans_max_value;
	trans_move = MoveNone;
	trans_retrieve(Trans, board->key, &trans_move, &trans_min_depth, &trans_max_depth, &trans_min_value,
				   &trans_max_value);

	note_moves(list, board, 0, trans_move);
	list->sort();
}

// search_full_root()

int_fast32_t search_full_root(list_t *list, board_t *board, int_fast32_t depth, int_fast32_t search_type) {

	ASSERT(list->is_ok());
	ASSERT(board_is_ok(board));
	ASSERT(depth_is_ok(depth));
	ASSERT(search_type == SearchNormal || search_type == SearchShort);

	ASSERT(list == SearchRoot->list);
	ASSERT(!LIST_IS_EMPTY(list));
	ASSERT(board == SearchCurrent->board);
	ASSERT(board_is_legal(board));
	ASSERT(depth >= 1);

	int_fast32_t a, b;
	if (SearchBest[SearchCurrent->multipv].value == 0) {
		a = -ValueInf;
		b = +ValueInf;
	} else {
		a = SearchBest[SearchCurrent->multipv].value - 40;
		b = SearchBest[SearchCurrent->multipv].value + 40;
	}

	if (SearchInput->multipv > 0) {
		a = -ValueInf;
		b = +ValueInf;
	}

	const int_fast32_t value = full_root(list, board, a, b, depth, 0, search_type);

	ASSERT(value_is_ok(value));
	ASSERT(LIST_VALUE(list, 0) == value);

	return value;
}

// full_root()

static int_fast32_t full_root(list_t *list, board_t *board, int_fast32_t alpha, int_fast32_t beta, int_fast32_t depth,
							  int_fast32_t height, int_fast32_t search_type) {

	ASSERT(list->is_ok());
	ASSERT(board_is_ok(board));
	ASSERT(range_is_ok(alpha, beta));
	ASSERT(depth_is_ok(depth));
	ASSERT(height_is_ok(height));
	ASSERT(search_type == SearchNormal || search_type == SearchShort);

	ASSERT(list == SearchRoot->list);
	ASSERT(!LIST_IS_EMPTY(list));
	ASSERT(board == SearchCurrent->board);
	ASSERT(board_is_legal(board));
	ASSERT(depth >= 1);

	// init
	SearchStack[height].best_move = MoveNone;
	SearchStack[height].move    = MoveNone;
	SearchStack[height].threat_move = MoveNone;
	SearchStack[height].reduced = false;

	SearchCurrent->node_nb++;
	SearchInfo->check_nb--;

	if (SearchCurrent->multipv == 0)
		for (int_fast32_t i = 0; i < LIST_SIZE(list); ++i)
			list->moves[i].value = ValueNone;

	int_fast32_t                         old_alpha = alpha;
	std::array<int_fast32_t, MultiPVMax> best_value;
	best_value[SearchCurrent->multipv] = ValueNone;

	// move loop
	for (int_fast32_t i = 0; i < LIST_SIZE(list); ++i) {

		int_fast32_t move = LIST_MOVE(list, i);

		if (SearchCurrent->multipv > 0) {
			bool              found = false;
			for (int_fast32_t j     = 0; j < SearchCurrent->multipv; ++j) {
				if (SearchBest[j].pv[0] == move) {
					found = true;
					break;
				}
			}
			if (found == true)
				continue;
		}

		SearchStack[height].move = move;

		SearchRoot->depth = depth;
		SearchRoot->move  = move;
		SearchRoot->move_pos = i;
		SearchRoot->move_nb = LIST_SIZE(list);

		search_update_root();
		int_fast32_t new_depth = full_new_depth(depth, move, board, board_is_check(board) && LIST_SIZE(list) == 1, true,
												height);

		undo_t undo[1]; //*?
		move_do(board, move, undo);

		int_fast32_t value;
		mv_t         new_pv[HeightMax];
		if (search_type == SearchShort || best_value[SearchCurrent->multipv] == ValueNone) { // first move
			value = -full_search(board, -beta, -alpha, new_depth, height + 1, new_pv, NodePV);
			if (value <= alpha) { // research
				old_alpha = -ValueInf;
				value = -full_search(board, -beta, ValueInf, new_depth, height + 1, new_pv, NodePV);
			} else if (value >= beta) { // research
				value = -full_search(board, -ValueInf, -alpha, new_depth, height + 1, new_pv, NodePV);
			}
		} else { // other moves
			value = -full_search(board, -alpha - 1, -alpha, new_depth, height + 1, new_pv, NodeCut);
			if (value > alpha) { // && value < beta
				SearchRoot->change = true;
				SearchRoot->easy = false;
				SearchRoot->flag = false;
				search_update_root();
				value = -full_search(board, -beta, -alpha, new_depth, height + 1, new_pv, NodePV);
			}
		}

		move_undo(board, move, undo);

		if (value <= alpha) { // upper bound
			list->moves[i].value = old_alpha;
		} else if (value >= beta) { // lower bound
			list->moves[i].value = beta;
		} else { // alpha < value < beta => exact value
			list->moves[i].value = value;
		}

		if (value > best_value[SearchCurrent->multipv] &&
			(best_value[SearchCurrent->multipv] == ValueNone || value > alpha)) {

			SearchBest[SearchCurrent->multipv].move = move;
			SearchStack[height].best_move = move;
			SearchBest[SearchCurrent->multipv].value = value;
			if (value <= alpha) { // upper bound
				SearchBest[SearchCurrent->multipv].flags = SearchUpper;
			} else if (value >= beta) { // lower bound
				SearchBest[SearchCurrent->multipv].flags = SearchLower;
			} else { // alpha < value < beta => exact value
				SearchBest[SearchCurrent->multipv].flags = SearchExact;
			}
			SearchBest[SearchCurrent->multipv].depth = depth;
			pv_cat(SearchBest[SearchCurrent->multipv].pv, new_pv, move);

			search_update_best();
		}

		if (value > best_value[SearchCurrent->multipv]) {
			best_value[SearchCurrent->multipv] = value;
			if (value > alpha) {
				if (search_type == SearchNormal) alpha = value;
				if (value >= beta) break;
			}
		}
	}

	ASSERT(value_is_ok(best_value));

	list->sort();

	ASSERT(SearchBest->move == LIST_MOVE(list, 0));
	ASSERT(SearchBest->value == best_value);

	if (best_value[SearchCurrent->multipv] > old_alpha && best_value[SearchCurrent->multipv] < beta)
		pv_fill(SearchBest[SearchCurrent->multipv].pv, board);

	return best_value[SearchCurrent->multipv];
}

// full_search()

static int_fast32_t full_search(board_t *board, int_fast32_t alpha, int_fast32_t beta, int_fast32_t depth,
								int_fast32_t height, mv_t pv[], int_fast32_t node_type) {

	ASSERT(board != nullptr);
	ASSERT(range_is_ok(alpha, beta));
	ASSERT(depth_is_ok(depth));
	ASSERT(height_is_ok(height));
	ASSERT(pv != nullptr);
	ASSERT(node_type == NodePV || node_type == NodeCut || node_type == NodeAll);
	ASSERT(board_is_legal(board));

	// horizon?
	if (depth <= 0) return full_quiescence(board, alpha, beta, 0, height, pv);

	// init
	SearchStack[height].best_move = MoveNone;
	SearchStack[height].move    = MoveNone;
	SearchStack[height].threat_move = MoveNone;
	SearchStack[height].reduced = false;
	//bool mate_threat = false;

	SearchCurrent->node_nb++;
	SearchInfo->check_nb--;
	PV_CLEAR(pv);

	if (height > SearchCurrent->max_depth) SearchCurrent->max_depth = height;

	if (SearchInfo->check_nb <= 0) {
		SearchInfo->check_nb += SearchInfo->check_inc;
		search_check();
	}

	// draw?
	if (board_is_repetition(board)/* || recog_draw(board)*/) return ValueDraw;

	// mate-distance pruning

	// lower bound
	int_fast32_t value = VALUE_MATE(height + 2); // does not work if the current position is mate
	if (value > alpha && board_is_mate(board)) value = VALUE_MATE(height);
	if (value > alpha) {
		alpha = value;
		if (value >= beta) return value;
	}

	// upper bound
	value = -VALUE_MATE(height + 1);

	if (value < beta) {
		beta = value;
		if (value <= alpha) return value;
	}


	// transposition table
	int_fast32_t trans_move = MoveNone;
	int_fast32_t trans_depth, trans_min_depth, trans_max_depth, trans_min_value, trans_max_value;
	if (depth >= TransDepth) {

		if (trans_retrieve(Trans, board->key, &trans_move, &trans_min_depth, &trans_max_depth, &trans_min_value,
						   &trans_max_value)) {

			// trans_move is now updated
			if (node_type != NodePV) {
				if (trans_min_value > +ValueEvalInf && trans_min_depth < depth)
					trans_min_depth = depth;

				if (trans_max_value < -ValueEvalInf && trans_max_depth < depth)
					trans_max_depth = depth;

				int_fast32_t min_value = -ValueInf;

				if (DEPTH_MATCH(trans_min_depth, depth)) {
					min_value = value_from_trans(trans_min_value, height);
					if (min_value >= beta) return min_value;
				}

				int_fast32_t max_value = +ValueInf;

				if (DEPTH_MATCH(trans_max_depth, depth)) {
					max_value = value_from_trans(trans_max_value, height);
					if (max_value <= alpha) return max_value;
				}

				if (min_value == max_value) return min_value; // exact match
			}
		}
	}

	// height limit
	if (height >= HeightMax - 1) return eval(board, alpha, beta, false, false);

	// more init
	int_fast32_t old_alpha = alpha, best_value = ValueNone, best_move = MoveNone, played_nb = 0;

	attack_t attack[1];
	attack_set(attack, board);
	bool in_check = ATTACK_IN_CHECK(attack);

	undo_t undo[1];
	mv_t   new_pv[HeightMax];
	bool   single_reply;
	int_fast32_t opt_value;
	std::array<mv_t, 256> played;
	sort_t sort[1];

	// null-move pruning
	if (UseNull && depth >= NullDepth && node_type != NodePV) {
		if (!in_check
			&& !value_is_mate(beta)
			&& do_null(board)
			&& (!UseNullEval || depth <= NullReduction + 1 || eval(board, alpha, beta, false, in_check) >= beta)) {

			// null-move search
			int_fast32_t new_depth = depth - NullReduction - 1;

			move_do_null(board, undo);
			value = -full_search(board, -beta, -beta + 1, new_depth, height + 1, new_pv, NODE_OPP(node_type));
			move_undo_null(board, undo);

			// verification search
			if (UseVer && depth > VerReduction) {
				if (value >= beta && (!UseVerEndgame || do_ver(board))) {
					new_depth = depth - VerReduction;
					ASSERT(new_depth > 0);

					int_fast32_t move;
					value = full_no_null(board, alpha, beta, new_depth, height, new_pv, NodeCut, trans_move, &move);

					if (value >= beta) {
						ASSERT(move == new_pv[0]);
						played[played_nb++] = move;
						best_move = move;
						SearchStack[height].move = move;
						SearchStack[height].best_move = move;
						best_value = value;
						pv_copy(pv, new_pv);
						goto cut;
					}
				}
			}

			// pruning
			if (value >= beta) {
				if (value > +ValueEvalInf) value = +ValueEvalInf; // do not return unproven mates
				ASSERT(!value_is_mate(value));

				// pv_cat(pv,new_pv,MoveNull);
				best_move  = MoveNone;
				best_value = value;
				goto cut;
			}

			SearchStack[height].threat_move = SearchStack[height + 1].best_move;
			if (SearchStack[height - 1].reduced) { // Idea by Tord Romstad
				if (/* value < alpha - 250 ||*/ MOVE_FROM(SearchStack[height + 1].best_move) ==
												MOVE_TO(SearchStack[height - 1].move))
					return alpha - 1;
				//depth++;
			}
		}
	}

		// mate threat
/*	mate_threat = false;
	if (value <= VALUE_MATE(height+2))  {
		mate_threat = true;
	} */
		// Razoring: idea by Tord Romstad (Glaurung)
	else if (node_type != NodePV && depth <= RazorDepth &&
			 eval(board, alpha, beta, false, in_check) < beta - RazorMargin) {
		value = full_quiescence(board, alpha, beta, 0, height, pv);
		if (value < beta)
			return value;
	}

	// Internal Iterative Deepening
	if (depth >= IIDDepth && node_type == NodePV && trans_move == MoveNone) {

		// new_depth = depth - IIDReduction;
		const int_fast32_t new_depth = std::min(depth - IIDReduction, depth / 2);
		ASSERT(new_depth > 0);

		value = full_search(board, alpha, beta, new_depth, height, new_pv, node_type);
		if (value <= alpha) value = full_search(board, -ValueInf, beta, new_depth, height, new_pv, node_type);

		trans_move = new_pv[0];
	}

	// move generation
	sort_init(sort, board, attack, depth, height, trans_move);

	single_reply = false;
	if (in_check && LIST_SIZE(sort->list) == 1) single_reply = true; // HACK

	// move loop
	opt_value = +ValueInf;

	int_fast32_t move;
	while ((move = sort_next(sort)) != MoveNone) {
		SearchStack[height].move = move;

		// extensions
		int_fast32_t new_depth = full_new_depth(depth, move, board, single_reply, node_type == NodePV, height);
		if (depth == 1 && opt_value == ValueInf && new_depth < depth)
			opt_value = eval(board, alpha, beta, false, in_check);

		// futility pruning
		if (UseFutility && depth <= FutilityPruningDepth && node_type != NodePV) {
			if (!in_check && new_depth < depth && !move_is_tactical(move, board) && !move_is_dangerous(move, board)) {

				ASSERT(!move_is_check(move, board));

				// optimistic evaluation
				if (opt_value == +ValueInf) {
					int_fast32_t FutilityMargin;
					if (depth == 2) {
						FutilityMargin = FutilityMargin2;
					} else if (depth == 3) {
						FutilityMargin = FutilityMargin3;
					} else {
						FutilityMargin = FutilityMargin1;
					}
					opt_value = eval(board, alpha, beta, false, in_check) + FutilityMargin;
					ASSERT(opt_value < +ValueInf);
				}

				value = opt_value;

				// pruning
				if (value <= alpha) {
					if (value > best_value) {
						best_value = value;
						PV_CLEAR(pv);
					}
					continue;
				}
			}
		}

		// history pruning
		bool reduced = false;
		value = sort->value; // history score
		if (!in_check && depth < SearchCurrent->max_extensions / 2 && node_type != NodePV && new_depth < depth &&
			value < 2 * HistoryValue / (depth + depth % 2))
			continue;

		if (UseHistory && depth >= HistoryDepth && node_type != NodePV) {
			if (!in_check && played_nb >= HistoryMoveNb && new_depth < depth) {
				ASSERT(best_value != ValueNone);
				ASSERT(played_nb > 0);
				ASSERT(sort->pos > 0 && move == LIST_MOVE(sort->list, sort->pos - 1));
				value = sort->value; // history score
				if (value < HistoryValue) {
					ASSERT(value >= 0 && value < 16384);
					ASSERT(move != trans_move);
					ASSERT(!move_is_tactical(move, board));
					ASSERT(!move_is_check(move, board));
					--new_depth;
					reduced = true;
					if (UseExtendedHistory && value < HistoryValue / 2 && depth >= 8) {
						--new_depth;
					}
				}
			}
		}

		// rebel reduction

		if (use_rebel_reduction && !in_check && !reduced && depth > 2 && new_depth < depth && node_type != NodePV &&
			!move_is_tactical(move, board) && !move_is_dangerous(move, board)) {

			int_fast32_t margin;
			switch (depth) {
				case 3:
					margin = 500;
					break;
				case 4:
					margin = 600;
					break;
				case 5:
					margin = 700;
					break;
				case 6:
					margin = 800;
					break;
				case 7:
					margin = 900;
					break;
				case 8:
					margin = 1000;
					break;
				default:
					margin = 1600;
					break;
			}

			// optimistic evaluation
			if (opt_value == +ValueInf) {
				opt_value = board->piece_material[White] - board->piece_material[Black];
				if (COLOUR_IS_BLACK(board->turn)) {
					opt_value = -opt_value;
				}
				ASSERT(opt_value < +ValueInf);
			}

			value = opt_value + margin;

			if (alpha > value) {
				--new_depth;
				reduced = true;
				//rebel_reduction = true;
			}

		}

		SearchStack[height].reduced = reduced;

		// recursive search
		move_do(board, move, undo);

		if (node_type != NodePV || best_value == ValueNone) { // first move
			value = -full_search(board, -beta, -alpha, new_depth, height + 1, new_pv, NODE_OPP(node_type));
		} else { // other moves
			value = -full_search(board, -alpha - 1, -alpha, new_depth, height + 1, new_pv, NodeCut);
			if (value > alpha) { // && value < beta
				value = -full_search(board, -beta, -alpha, new_depth, height + 1, new_pv, NodePV);
			}
		}

		// history-pruning re-search
		if (research_on_beta) {
			if (reduced && value >= beta) {

				ASSERT(node_type != NodePV);

				SearchStack[height].reduced = false;
				++new_depth;
				ASSERT(new_depth == depth - 1);

				value = -full_search(board, -beta, -alpha, new_depth, height + 1, new_pv, NODE_OPP(node_type));
			}
		} else {
			if (reduced && value > alpha) {

				ASSERT(node_type != NodePV);

				SearchStack[height].reduced = false;
				++new_depth;
				ASSERT(new_depth == depth - 1);

				value = -full_search(board, -beta, -alpha, new_depth, height + 1, new_pv, NODE_OPP(node_type));
			}
		}

		move_undo(board, move, undo);

		played[played_nb++] = move;

		if (value > best_value) {
			best_value = value;
			pv_cat(pv, new_pv, move);
			if (value > alpha) {
				alpha = value;
				best_move = move;
				SearchStack[height].best_move = move;
				if (value >= beta) {
					goto cut;
				}
			}
		}

		if (node_type == NodeCut) node_type = NodeAll;
	}

	// ALL node

	if (best_value == ValueNone) { // no legal move
		if (in_check) {
			ASSERT(board_is_mate(board));
			return VALUE_MATE(height);
		} else {
			ASSERT(board_is_stalemate(board));
			return ValueDraw;
		}
	}

	cut: //refactor?

	ASSERT(value_is_ok(best_value));

	// move ordering
	if (best_move != MoveNone) {

		good_move(best_move, board, depth, height);
		if (best_value >= beta && !move_is_tactical(best_move, board)) {

			ASSERT(played_nb > 0 && played[played_nb - 1] == best_move);

			for (int_fast32_t i = 0; i < played_nb - 1; ++i) {
				const int_fast32_t move = played[i];
				ASSERT(move != best_move);
				history_bad(move, board);
			}

			history_good(best_move, board);
		}
	}

	// transposition table

	if (depth >= TransDepth) {

		trans_move      = best_move;
		trans_depth     = depth;
		trans_min_value = (best_value > old_alpha) ? value_to_trans(best_value, height) : -ValueInf;
		trans_max_value = (best_value < beta) ? value_to_trans(best_value, height) : +ValueInf;

		trans_store(Trans, board->key, trans_move, trans_depth, trans_min_value, trans_max_value);
	}

	return best_value;
}

// full_no_null()

static int_fast32_t full_no_null(board_t *board, int_fast32_t alpha, int_fast32_t beta, int_fast32_t depth,
								 int_fast32_t height, mv_t pv[], int_fast32_t node_type, int_fast32_t trans_move,
								 int_fast32_t *best_move) {

	ASSERT(board != nullptr);
	ASSERT(range_is_ok(alpha, beta));
	ASSERT(depth_is_ok(depth));
	ASSERT(height_is_ok(height));
	ASSERT(pv != nullptr);
	ASSERT(node_type == NodePV || node_type == NodeCut || node_type == NodeAll);
	ASSERT(trans_move == MoveNone || move_is_ok(trans_move));
	ASSERT(best_move != nullptr);

	ASSERT(board_is_legal(board));
	ASSERT(!board_is_check(board));
	ASSERT(depth >= 1);

	// init
	SearchStack[height].best_move = MoveNone;
	SearchStack[height].move    = MoveNone;
	SearchStack[height].threat_move = MoveNone;
	SearchStack[height].reduced = false;

	SearchCurrent->node_nb++;
	SearchInfo->check_nb--;
	PV_CLEAR(pv);

	if (height > SearchCurrent->max_depth) SearchCurrent->max_depth = height;

	if (SearchInfo->check_nb <= 0) {
		SearchInfo->check_nb += SearchInfo->check_inc;
		search_check();
	}

	attack_t attack[1];
	attack_set(attack, board);
	ASSERT(!ATTACK_IN_CHECK(attack));

	*best_move = MoveNone;
	int_fast32_t best_value = ValueNone;

	// move loop

	sort_t sort[1];
	sort_init(sort, board, attack, depth, height, trans_move);

	int_fast32_t move;
	while ((move = sort_next(sort)) != MoveNone) {

		SearchStack[height].move = move;

		const int_fast32_t new_depth = full_new_depth(depth, move, board, false, false, height);

		undo_t undo[1];
		mv_t               new_pv[HeightMax];

		move_do(board, move, undo);
		int_fast32_t value = -full_search(board, -beta, -alpha, new_depth, height + 1, new_pv, NODE_OPP(node_type));
		move_undo(board, move, undo);

		if (value > best_value) {
			best_value = value;
			pv_cat(pv, new_pv, move);
			if (value > alpha) {
				alpha = value;
				*best_move = move;
				SearchStack[height].best_move = move;
				if (value >= beta) goto cut;
			}
		}
	}

	// ALL node

	if (best_value == ValueNone) { // no legal move => stalemate
		ASSERT(board_is_stalemate(board));
		best_value = ValueDraw;
	}

	cut: //remove me?

	ASSERT(value_is_ok(best_value));
	return best_value;
}

// full_quiescence()

static int_fast32_t full_quiescence(board_t *board, int_fast32_t alpha, int_fast32_t beta, int_fast32_t depth,
									int_fast32_t height, mv_t pv[]) {

	ASSERT(board != nullptr);
	ASSERT(range_is_ok(alpha, beta));
	ASSERT(depth_is_ok(depth));
	ASSERT(height_is_ok(height));
	ASSERT(pv != nullptr);

	ASSERT(board_is_legal(board));
	ASSERT(depth <= 0);

	// init
	SearchStack[height].best_move = MoveNone;
	SearchStack[height].move    = MoveNone;
	SearchStack[height].threat_move = MoveNone;
	SearchStack[height].reduced = false;

	SearchCurrent->node_nb++;
	SearchInfo->check_nb--;
	PV_CLEAR(pv);

	if (height > SearchCurrent->max_depth) SearchCurrent->max_depth = height;

	if (SearchInfo->check_nb <= 0) {
		SearchInfo->check_nb += SearchInfo->check_inc;
		search_check();
	}

	// draw?
	if (board_is_repetition(board)/* || recog_draw(board)*/) return ValueDraw;

	// mate-distance pruning

	// lower bound
	int_fast32_t value = VALUE_MATE(height + 2); // does not work if the current position is mate

	if (value > alpha && board_is_mate(board)) value = VALUE_MATE(height);
	if (value > alpha) {
		alpha = value;
		if (value >= beta) return value;
	}

	// upper bound
	value = -VALUE_MATE(height + 1);

	if (value < beta) {
		beta = value;
		if (value <= alpha) return value;
	}


	// more init
	attack_t attack[1];
	attack_set(attack, board);
	bool in_check = ATTACK_IN_CHECK(attack);

	if (in_check) {
		ASSERT(depth < 0);
		++depth; // in-check extension
	}

	// height limit
	if (height >= HeightMax - 1) return eval(board, alpha, beta, false, false);

	// more init
	int_fast32_t old_alpha = alpha, best_value = ValueNone, best_move = MoveNone;

	/* if (UseDelta) */ int_fast32_t opt_value = +ValueInf;
	sort_t       sort[1];

	if (!in_check) {

		// lone-king stalemate?
		if (simple_stalemate(board)) return ValueDraw;

		// stand pat
		value = eval(board, alpha, beta, true, in_check);

		ASSERT(value > best_value);
		best_value = value;
		if (value > alpha) {
			alpha = value;
			if (value >= beta) goto cut;
		}

		if (UseDelta) {
			opt_value = value + DeltaMargin;
			ASSERT(opt_value < +ValueInf);
		}
	}

	// move loop
	sort_init_qs(sort, board, attack, depth >= CheckDepth);

	int_fast32_t move;
	while ((move = sort_next_qs(sort)) != MoveNone) {
		SearchStack[height].move = move;

		// delta pruning
		if (UseDelta && beta == old_alpha + 1) {
			if (!in_check && !move_is_check(move, board) && !capture_is_dangerous(move, board)) {
				ASSERT(move_is_tactical(move, board));

				// optimistic evaluation
				value = opt_value;

				int_fast32_t to = MOVE_TO(move), capture = board->square[to];

				if (capture != Empty) {
					value += VALUE_PIECE(capture);
				} else if (MOVE_IS_EN_PASSANT(move)) {
					value += ValuePawn;
				}

				if (MOVE_IS_PROMOTE(move)) value += ValueQueen - ValuePawn;

				// pruning
				if (value <= alpha) {
					if (value > best_value) {
						best_value = value;
						PV_CLEAR(pv);
					}

					continue;
				}
			}
		}

		undo_t undo[1];
		mv_t new_pv[HeightMax];

		move_do(board, move, undo);
		value = -full_quiescence(board, -beta, -alpha, depth - 1, height + 1, new_pv);
		move_undo(board, move, undo);

		if (value > best_value) {
			best_value = value;
			pv_cat(pv, new_pv, move);
			if (value > alpha) {
				alpha = value;
				best_move = move;
				SearchStack[height].best_move = move;
				if (value >= beta) goto cut;
			}
		}
	}

	// ALL node
	if (best_value == ValueNone) { // no legal move
		ASSERT(board_is_mate(board));
		return VALUE_MATE(height);
	}

	cut:

	ASSERT(value_is_ok(best_value));
	return best_value;
}

// full_new_depth()

static int_fast32_t full_new_depth(int_fast32_t depth, int_fast32_t move, board_t *board, bool single_reply, bool in_pv,
								   int_fast32_t height) {

	ASSERT(depth_is_ok(depth));
	ASSERT(move_is_ok(move));
	ASSERT(board != nullptr);
	ASSERT(single_reply == true || single_reply == false);
	ASSERT(in_pv == true || in_pv == false);

	ASSERT(depth > 0);

	int_fast32_t new_depth = depth - 1;

	if (SearchCurrent->max_extensions > height) {
		if ((single_reply)
			|| (in_pv && MOVE_TO(move) == board->cap_sq // recapture
				&& (see_move(move, board) > 0 ||
					abs(VALUE_PIECE(board->square[MOVE_TO(move)]) - VALUE_PIECE(board->square[MOVE_FROM(move)])) <=
					250))
			|| (in_pv && PIECE_IS_PAWN(MOVE_PIECE(move, board))
				&& PAWN_RANK(MOVE_TO(move), board->turn) == Rank7
				/* && see_move(move,board) >= 0 */)
			|| move_is_check(move, board)) {
			++new_depth;
		}
	}

	ASSERT(new_depth >= 0 && new_depth <= depth);
	return new_depth;
}

// do_null()

static bool do_null(const board_t *board) {

	ASSERT(board != nullptr);
	// use null move if the side-to-move has at least one piece
	return board->piece[board->turn].size() >= 2; // king + one piece
}

// do_ver()

static bool do_ver(const board_t *board) {

	ASSERT(board != nullptr);
	// use verification if the side-to-move has at most one piece
	return board->piece[board->turn].size() <= 3; // king + one piece was 2
}

// pv_fill()

static void pv_fill(const mv_t pv[], board_t *board) {

	ASSERT(pv != nullptr);
	ASSERT(board != nullptr);

	const int_fast32_t move = *pv;

	if (move != MoveNone && move != MoveNull) {

		undo_t undo[1];
		move_do(board, move, undo);
		pv_fill(pv + 1, board);
		move_undo(board, move, undo);

		int_fast32_t trans_move = move,
					 trans_depth = -127, // HACK
					 trans_min_value = -ValueInf,
					 trans_max_value = +ValueInf;

		trans_store(Trans, board->key, trans_move, trans_depth, trans_min_value, trans_max_value);
	}
}

// move_is_dangerous()

static bool move_is_dangerous(int_fast32_t move, const board_t *board) {

	ASSERT(move_is_ok(move));
	ASSERT(board != nullptr);
	ASSERT(!move_is_tactical(move, board));

	int_fast32_t piece = MOVE_PIECE(move, board);

	if (PIECE_IS_PAWN(piece)
		&& PAWN_RANK(MOVE_TO(move), board->turn) >= Rank7)
		return true;

	return false;
}

// capture_is_dangerous()

static bool capture_is_dangerous(int_fast32_t move, const board_t *board) {

	ASSERT(move_is_ok(move));
	ASSERT(board != nullptr);
	ASSERT(move_is_tactical(move, board));

	int_fast32_t piece = MOVE_PIECE(move, board);

	if (PIECE_IS_PAWN(piece)
		&& PAWN_RANK(MOVE_TO(move), board->turn) >= Rank7)
		return true;

	int_fast32_t capture = move_capture(move, board);

	if (PIECE_IS_QUEEN(capture)) return true;

	if (PIECE_IS_PAWN(capture)
		&& PAWN_RANK(MOVE_TO(move), board->turn) <= Rank2)
		return true;

	return false;
}

// simple_stalemate()

static bool simple_stalemate(const board_t *board) {

	ASSERT(board != nullptr);
	ASSERT(board_is_legal(board));
	ASSERT(!board_is_check(board));

	// lone king?
	const int_fast32_t me = board->turn;
	if (board->piece[me].size() != 1 || board->pawn[me].size() != 0) return false; // no

	// king in a corner?
	const int_fast32_t king = KING_POS(board, me);
	if (king != A1 && king != H1 && king != A8 && king != H8) return false; // no

	// init
	const int_fast32_t opp = COLOUR_OPP(me), opp_flag = COLOUR_FLAG(opp);

	// king can move?
	const int_fast32_t from   = king;

	int_fast32_t     inc;
	for (const inc_t *inc_ptr = KingInc.data(); (inc = *inc_ptr) != IncNone; ++inc_ptr) {
		const int_fast32_t to = from + inc, capture = board->square[to];
		if (capture == Empty || FLAG_IS(capture, opp_flag)) {
			if (!is_attacked(board, to, opp)) return false; // legal king move
		}
	}

	// no legal move
	ASSERT(board_is_stalemate((board_t *) board));
	return true;
}

// end of search_full.cpp
