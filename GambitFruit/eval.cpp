// eval.cpp

// includes

#include <cstdlib>
#include "attack.h"
#include "eval.h"
#include "material.h"
#include "move.h"
#include "option.h"
#include "pawn.h"
#include "see.h"
#include "value.h"
#include "move_gen.h"
#include "scorpio_egbb.h"

// macros

#define THROUGH(piece) ((piece)==Empty)

// constants and variables

const std::array<std::array<S8, 256>, 2> KnightOutpostMatrix =
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 2, 5, 10, 10, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 5, 10, 10, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
	 5, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 5, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 2, 5, 10, 10, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 5, 10, 10, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};

static S32 lazy_eval_cutoff = 50;
static bool KingSafety = false; // true
//static S32 KingSafetyMargin = 1600;
static bool king_is_safe[ColourNb];

static /* constexpr */ S16 PieceActivityWeight = 256; // 100%
static /* constexpr */ S16 ShelterOpening = 256; // 100%
//static /* constexpr */ S16 KingSafetyWeight    = 256; // 100%
static /* constexpr */ S16 PassedPawnWeight = 256; // 100%

static constexpr S8 MobMove = 1;
static constexpr S8 MobAttack = 1;
static constexpr S8 MobDefense = 0;

static constexpr std::array<S8, 9> knight_mob = {-16, -12, -8, -4, 0, 4, 8, 12, 16};
static constexpr std::array<S8, 14> bishop_mob = {-30, -25, -20, -15, -10, -5, 0, 5, 10, 15, 20, 25, 30, 35};
static constexpr std::array<S8, 15> rook_mob_open = {-14, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14};
static constexpr std::array<S8, 15> rook_mob_end = {-28, -24, -20, -16, -12, -8, -4, 0, 4, 8, 12, 16, 20, 24, 28};
static constexpr std::array<S8, 27> queen_mob_open =
	{-13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
static constexpr std::array<S8, 27> queen_mob_end =
	{-26, -24, -22, -20, -18, -16, -14, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26};

static constexpr std::array<S8, 9> pawns_on_bishop_colour_opening = {9, 6, 3, 0, -3, -6, -9, -12, -15};
static constexpr std::array<S8, 9> pawns_on_bishop_colour_endgame = {12, 8, 4, 0, -4, -8, -12, -16, -20};

static /* constexpr */ std::array<S8, 9> PawnAmountBonusOpening = {-10, -9, -6, 0, 5, 5, 5, 2, -4};
static /* constexpr */ std::array<S8, 9> PawnAmountBonusEndgame = {-55, -20, -2, 0, 10, 8, 5, 3, -8};

static constexpr S8 RookSemiOpenFileOpening = 10;
static constexpr S8 RookSemiOpenFileEndgame = 10;
static constexpr S8 RookOpenFileOpening = 20;
static constexpr S8 RookOpenFileEndgame = 20;
static constexpr S8 RookSemiKingFileOpening = 10;
static constexpr S8 RookKingFileOpening = 20;
static constexpr S8 RookOnBadPawnFileOpening = 8;
static constexpr S8 RookOnBadPawnFileEndgame = 8;

static /* constexpr */ S16 KingAttackOpening = 20; // was 20

static constexpr S8 knight_tropism_opening = 3;
static constexpr S8 bishop_tropism_opening = 2;
static constexpr S8 rook_tropism_opening = 2;
static constexpr S8 queen_tropism_opening = 2;

static constexpr S8 knight_tropism_endgame = 3;
static constexpr S8 bishop_tropism_endgame = 1;
static constexpr S8 rook_tropism_endgame = 1;
static constexpr S8 queen_tropism_endgame = 4;

static /* constexpr */ S16 StormOpening = 10;

static constexpr S8 Rook7thOpening = 20;
static constexpr S8 Rook7thEndgame = 40;
static constexpr S8 Queen7thOpening = 10;
static constexpr S8 Queen7thEndgame = 20;

static constexpr S16 TrappedBishop = 100;

static constexpr S8 BlockedBishop = 50;
static constexpr S8 BlockedRook = 50;
static constexpr S8 BlockedCenterPawn = 10;

static constexpr S8 PassedOpeningMin = 10;
static constexpr S8 PassedOpeningMax = 70;
static constexpr S8 PassedEndgameMin = 20;
static constexpr S16 PassedEndgameMax = 140;

static constexpr S16 UnstoppablePasser = 800;
static constexpr S8 FreePasser = 60;

static constexpr S8 AttackerDistance = 5;
static constexpr S8 DefenderDistance = 20;

// "constants"

static constexpr std::array<S16, 16>
	KingAttackWeight = {0, 0, 128, 192, 224, 240, 248, 252, 254, 255, 256, 256, 256, 256, 256, 256,};


// variables

static S32 MobUnit[ColourNb][PieceNb];

static S32 KingAttackUnit[PieceNb];

// prototypes

static void eval_draw(const board_t *board, const material_info_t *mat_info, const pawn_info_t *pawn_info, S32 mul[2]);

static void eval_piece
	(const board_t *board, const material_info_t *mat_info, const pawn_info_t *pawn_info, S32 *opening, S32 *endgame);
static void eval_king(const board_t *board, const material_info_t *mat_info, S32 *opening, S32 *endgame);
static void eval_passer(const board_t *board, const pawn_info_t *pawn_info, S32 *opening, S32 *endgame);
static void eval_pattern(const board_t *board, S32 *opening, S32 *endgame);

static bool unstoppable_passer(const board_t *board, S32 pawn, S8 colour);
static bool king_passer(const board_t *board, S32 pawn, S8 colour);
static bool free_passer(const board_t *board, S32 pawn, S8 colour);

static S32 pawn_att_dist(S32 pawn, S32 king, S8 colour);
static S32 pawn_def_dist(S32 pawn, S32 king, S8 colour);

static void draw_init_list(S32 list[], const board_t *board, S32 pawn_colour);

static bool draw_krpkr(const S32 *list);
static bool draw_kbpkb(const S32 *list);

static S32 shelter_square(const board_t *board, S32 square, S8 colour);
static S8 shelter_file(const board_t *board, S32 file, S32 rank, S8 colour);

static S16 storm_file(const board_t *board, S32 file, S8 colour);

static bool bishop_can_attack(const board_t *board, S32 to, S8 colour);

// functions

// eval_init()

void eval_init() {

	// UCI options

	PieceActivityWeight = (option_get_int("Piece Activity") * 256 + 50) / 100;
	//KingSafetyWeight	 = (option_get_int("King Safety")	 * 256 + 50) / 100;
	PassedPawnWeight = (option_get_int("Passed Pawns") * 256 + 50) / 100;
	ShelterOpening = (option_get_int("Pawn Shelter") * 256 + 50) / 100;
	StormOpening = (10 * option_get_int("Pawn Storm")) / 100;
	KingAttackOpening = (20 * option_get_int("King Attack")) / 100;

#ifdef TMO_OPT
	//test
	PawnAmountBonusOpening[0] = option_get_int("PawnAmountBonusOpening0");
	PawnAmountBonusEndgame[0] = option_get_int("PawnAmountBonusEndgame0");
	PawnAmountBonusOpening[1] = option_get_int("PawnAmountBonusOpening1");
	PawnAmountBonusEndgame[1] = option_get_int("PawnAmountBonusEndgame1");
	PawnAmountBonusOpening[2] = option_get_int("PawnAmountBonusOpening2");
	PawnAmountBonusEndgame[2] = option_get_int("PawnAmountBonusEndgame2");
	PawnAmountBonusOpening[3] = option_get_int("PawnAmountBonusOpening3");
	PawnAmountBonusEndgame[3] = option_get_int("PawnAmountBonusEndgame3");
	PawnAmountBonusOpening[4] = option_get_int("PawnAmountBonusOpening4");
	PawnAmountBonusEndgame[4] = option_get_int("PawnAmountBonusEndgame4");
	PawnAmountBonusOpening[5] = option_get_int("PawnAmountBonusOpening5");
	PawnAmountBonusEndgame[5] = option_get_int("PawnAmountBonusEndgame5");
	PawnAmountBonusOpening[6] = option_get_int("PawnAmountBonusOpening6");
	PawnAmountBonusEndgame[6] = option_get_int("PawnAmountBonusEndgame6");
	PawnAmountBonusOpening[7] = option_get_int("PawnAmountBonusOpening7");
	PawnAmountBonusEndgame[7] = option_get_int("PawnAmountBonusEndgame7");
	PawnAmountBonusOpening[8] = option_get_int("PawnAmountBonusOpening8");
	PawnAmountBonusEndgame[8] = option_get_int("PawnAmountBonusEndgame8");
#endif

	if (option_get_int("Chess Knowledge") == 500) lazy_eval_cutoff = ValueEvalInf; else
		lazy_eval_cutoff = (50 * option_get_int("Chess Knowledge")) / 100;

	// mobility table

	for (S8 colour = 0; colour < ColourNb; ++colour) {
		for (S16 piece = 0; piece < PieceNb; ++piece) {
			MobUnit[colour][piece] = 0;
		}
	}

	MobUnit[White][Empty] = MobMove;

	MobUnit[White][BP] = MobAttack;
	MobUnit[White][BN] = MobAttack;
	MobUnit[White][BB] = MobAttack;
	MobUnit[White][BR] = MobAttack;
	MobUnit[White][BQ] = MobAttack;
	MobUnit[White][BK] = MobAttack;

	MobUnit[White][WP] = MobDefense;
	MobUnit[White][WN] = MobDefense;
	MobUnit[White][WB] = MobDefense;
	MobUnit[White][WR] = MobDefense;
	MobUnit[White][WQ] = MobDefense;
	MobUnit[White][WK] = MobDefense;

	MobUnit[Black][Empty] = MobMove;

	MobUnit[Black][WP] = MobAttack;
	MobUnit[Black][WN] = MobAttack;
	MobUnit[Black][WB] = MobAttack;
	MobUnit[Black][WR] = MobAttack;
	MobUnit[Black][WQ] = MobAttack;
	MobUnit[Black][WK] = MobAttack;

	MobUnit[Black][BP] = MobDefense;
	MobUnit[Black][BN] = MobDefense;
	MobUnit[Black][BB] = MobDefense;
	MobUnit[Black][BR] = MobDefense;
	MobUnit[Black][BQ] = MobDefense;
	MobUnit[Black][BK] = MobDefense;

	// KingAttackUnit[]

	for (S16 piece = 0; piece < PieceNb; ++piece) {
		KingAttackUnit[piece] = 0;
	}

	KingAttackUnit[WN] = 1;
	KingAttackUnit[WB] = 1;
	KingAttackUnit[WR] = 2;
	KingAttackUnit[WQ] = 4;

	KingAttackUnit[BN] = 1;
	KingAttackUnit[BB] = 1;
	KingAttackUnit[BR] = 2;
	KingAttackUnit[BQ] = 4;
}

// eval()

S32 eval(/*const*/ board_t *board, S32 alpha, bool do_le, bool in_check) {

	ASSERT(board != NULL);

	ASSERT(board_is_legal(board));
	//ASSERT(!board_is_check(board)); // exceptions are extremely rare //TODO: check already no???

	if (egbb_is_loaded) {
		if (board->piece_nb <= bitbase_pieces) {
			S32 eval;
			if (bitbase_probe(board, eval))
				return eval;
		}
	}

	// material
	material_info_t mat_info[1];
	material_get_info(mat_info, board);

	S32 opening = mat_info->opening, endgame = mat_info->endgame;

	S32 mul[ColourNb];
	mul[White] = mat_info->mul[White];
	mul[Black] = mat_info->mul[Black];

	// PST
	opening += board->opening;
	endgame += board->endgame;

	// draw
	pawn_info_t pawn_info[1];
	pawn_get_info(pawn_info, board);
	eval_draw(board, mat_info, pawn_info, mul);

	if (mat_info->mul[White] < mul[White]) mul[White] = mat_info->mul[White];
	if (mat_info->mul[Black] < mul[Black]) mul[Black] = mat_info->mul[Black];

	if (mul[White] == 0 && mul[Black] == 0) return ValueDraw;

	bool is_cut = false;

	const S32 phase = mat_info->phase;
	S32 lazy_eval = ((opening * (256 - mat_info->phase)) + (endgame * mat_info->phase)) / 256;

	if (COLOUR_IS_BLACK(board->turn)) lazy_eval = -lazy_eval;

	/* lazy Cutoff */
	if (do_le && !in_check && board->piece[White].size() > 2 && board->piece[Black].size() > 2) {

		/*ASSERT(eval >= -ValueEvalInf && eval <= +ValueEvalInf); //eval is not defined yet (?)

		if (lazy_eval - lazy_eval_cutoff >= beta)
			return (lazy_eval);*/
		if (lazy_eval + board->pvalue + lazy_eval_cutoff <= alpha) {
			//return (lazy_eval+board->pvalue);
			is_cut = true;
			goto cut;
		}
		// end lazy cuttoff
	}
	// pawns
	opening += pawn_info->opening;
	endgame += pawn_info->endgame;

	// eval
	eval_piece(board, mat_info, pawn_info, &opening, &endgame);
	eval_king(board, mat_info, &opening, &endgame);
	eval_passer(board, pawn_info, &opening, &endgame);
	eval_pattern(board, &opening, &endgame);

	//TODO: If you are better no pawns in endgame is bad. More pawn is better

	if (opening > ValueDraw)
		opening += PawnAmountBonusOpening[board->pawn[White].size()];
	else
		opening -= PawnAmountBonusOpening[board->pawn[Black].size()];

	if (endgame > ValueDraw)
		endgame += PawnAmountBonusEndgame[board->pawn[White].size()];
	else
		endgame -= PawnAmountBonusEndgame[board->pawn[Black].size()];

	cut:

	// phase mix
	S32 eval = ((opening * (256 - phase)) + (endgame * phase)) / 256;

	// drawish bishop endgames
	if ((mat_info->flags & DrawBishopFlag) != 0) {

		S16 wb = board->piece[White][1], bb = board->piece[Black][1];
		ASSERT(PIECE_IS_BISHOP(board->square[wb]));
		ASSERT(PIECE_IS_BISHOP(board->square[bb]));

		if (SQUARE_COLOUR(wb) != SQUARE_COLOUR(bb)) {
			if (mul[White] == 16) mul[White] = 8; // 1/2
			if (mul[Black] == 16) mul[Black] = 8; // 1/2
		}
	}

	// draw bound
	if (eval > ValueDraw) {
		eval = (eval * mul[White]) / 16;
	} else if (eval < ValueDraw) {
		eval = (eval * mul[Black]) / 16;
	}

	// value range
	if (eval < -ValueEvalInf) eval = -ValueEvalInf;
	if (eval > +ValueEvalInf) eval = +ValueEvalInf;

	ASSERT(eval >= -ValueEvalInf && eval <= +ValueEvalInf);

	// turn
	if (COLOUR_IS_BLACK(board->turn)) eval = -eval;
	if (!is_cut) board->pvalue = abs(eval - lazy_eval);

	ASSERT(!value_is_mate(eval));

	// Tempo
	S8 tempo = 10;//((10 * (256 - phase)) + (20 * phase)) / 256;

	// Tempo draw bound
	if (COLOUR_IS_WHITE(board->turn)) {
		if (eval > ValueDraw) {
			tempo = (tempo * mul[White]) / 16;
		} else if (eval < ValueDraw) {
			tempo = (tempo * mul[Black]) / 16;
		}
	} else {
		if (eval < ValueDraw) {
			tempo = (tempo * mul[White]) / 16;
		} else if (eval > ValueDraw) {
			tempo = (tempo * mul[Black]) / 16;
		}
	}

	return (eval + tempo);
}

// eval_draw()

static void eval_draw(const board_t *board, const material_info_t *mat_info, const pawn_info_t *pawn_info, S32 mul[2]) {

	ASSERT(board != NULL);
	ASSERT(mat_info != NULL);
	ASSERT(pawn_info != NULL);
	ASSERT(mul != NULL);

	// draw patterns

	for (S8 colour = 0; colour < ColourNb; ++colour) {

		const S8 me = colour, opp = COLOUR_OPP(me);

		// KB*P+K* draw
		if ((mat_info->cflags[me] & MatRookPawnFlag) != 0) {

			const S16 pawn = pawn_info->single_file[me];

			if (pawn != SquareNone) { // all pawns on one file

				const S8 pawn_file = SQUARE_FILE(pawn);
				if (pawn_file == FileA || pawn_file == FileH) {

					const S16 king = KING_POS(board, opp), prom = PAWN_PROMOTE(pawn, me);

					if (DISTANCE(king, prom) <= 1 && !bishop_can_attack(board, prom, me)) {
						mul[me] = 0;
					}
				}
			}
		}

		// K(B)P+K+ draw
		if ((mat_info->cflags[me] & MatBishopFlag) != 0) {

			const S16 pawn = pawn_info->single_file[me];

			if (pawn != SquareNone) { // all pawns on one file

				const S16 king = KING_POS(board, opp);

				if (SQUARE_FILE(king) == SQUARE_FILE(pawn) && PAWN_RANK(king, me) > PAWN_RANK(pawn, me)
					&& !bishop_can_attack(board, king, me)) {
					mul[me] = 1; // 1/16
				}
			}
		}

		// KNPK* draw
		if ((mat_info->cflags[me] & MatKnightFlag) != 0) {

			const S16 pawn = board->pawn[me][0], king = KING_POS(board, opp);

			if (SQUARE_FILE(king) == SQUARE_FILE(pawn) && PAWN_RANK(king, me) > PAWN_RANK(pawn, me)
				&& PAWN_RANK(pawn, me) <= Rank6) {
				mul[me] = 1; // 1/16
			}
		}
	}

	// recognisers, only heuristic draws here!
	if (false) {
	} else if (mat_info->recog == MAT_KRPKR) {

		S32 list[7 + 1];
		// KRPKR (white)
		draw_init_list(list, board, White);

		if (draw_krpkr(list)) {
			mul[White] = 1; // 1/16;
			mul[Black] = 1; // 1/16;
		}
	} else if (mat_info->recog == MAT_KRKRP) {

		S32 list[7 + 1];
		// KRPKR (black)
		draw_init_list(list, board, Black);

		if (draw_krpkr(list)) {
			mul[White] = 1; // 1/16;
			mul[Black] = 1; // 1/16;
		}
	} else if (mat_info->recog == MAT_KBPKB) {

		S32 list[7 + 1];
		// KBPKB (white)
		draw_init_list(list, board, White);

		if (draw_kbpkb(list)) {
			mul[White] = 1; // 1/16;
			mul[Black] = 1; // 1/16;
		}
	} else if (mat_info->recog == MAT_KBKBP) {

		S32 list[7 + 1];
		// KBPKB (black)
		draw_init_list(list, board, Black);

		if (draw_kbpkb(list)) {
			mul[White] = 1; // 1/16;
			mul[Black] = 1; // 1/16;
		}
	}
}

// eval_piece()

static void eval_piece(const board_t *board,
                       const material_info_t *mat_info,
                       const pawn_info_t *pawn_info,
                       S32 *opening,
                       S32 *endgame) {

	ASSERT(board != NULL);
	ASSERT(mat_info != NULL);
	ASSERT(pawn_info != NULL);
	ASSERT(opening != NULL);
	ASSERT(endgame != NULL);

	// init
	S32 op[ColourNb], eg[ColourNb];
	op[0] = op[1] = eg[0] = eg[1] = 0;

	// eval

	for (S8 colour = 0; colour < ColourNb; ++colour) {

		const S8 me = colour, opp = COLOUR_OPP(me);
		//const S32 opp_flag = COLOUR_FLAG(opp);
		const S32 *unit = MobUnit[me];

		// piece loop
		for (auto from = board->piece[me].begin() + 1; from != board->piece[me].end(); ++from) { // HACK: no king

			const S16 piece = board->square[*from];

			const S16 king = KING_POS(board, opp);
			const S8 king_file = SQUARE_FILE(king), king_rank = SQUARE_RANK(king);
			const S8 piece_file = SQUARE_FILE(*from), piece_rank = SQUARE_RANK(*from);
			switch (PIECE_TYPE(piece)) {

				case Knight64: {

					// mobility
					S32 mob = 0;

					for (U8 i = 0; i < knight_moves.size(); ++i)
						mob += unit[board->square[*from + knight_moves[i]]];

					op[me] += knight_mob[mob];
					eg[me] += knight_mob[mob];

					op[me] += (knight_tropism_opening * 8)
						- (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * knight_tropism_opening));
					eg[me] += (knight_tropism_endgame * 8)
						- (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * knight_tropism_endgame));


					// outpost
					S16 out_mob = 0;
					if (me == White) {
						if (board->square[*from - 17] == WP)
							out_mob += KnightOutpostMatrix[me][*from];
						if (board->square[*from - 15] == WP)
							out_mob += KnightOutpostMatrix[me][*from];
					} else {
						if (board->square[*from + 17] == BP)
							out_mob += KnightOutpostMatrix[me][*from];
						if (board->square[*from + 15] == BP)
							out_mob += KnightOutpostMatrix[me][*from];
					}

					op[me] += out_mob;
					// eg[me] += out_mob;

					break;
				}

				case Bishop64: {

					// mobility
					S32 mob = 0;
					S16 capture;

					for (U8 i = 0; i < bishop_moves.size(); ++i) {
						for (S16 to = *from + bishop_moves[i]; capture = board->square[to], THROUGH(capture);
						     to += bishop_moves[i])
							mob += MobMove;
						mob += unit[capture];
					}

					op[me] += bishop_mob[mob];
					eg[me] += bishop_mob[mob];

					op[me] += (bishop_tropism_opening * 8)
						- (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * bishop_tropism_opening));
					eg[me] += (bishop_tropism_endgame * 8)
						- (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * bishop_tropism_endgame));

					if (SQUARE_COLOUR(*from) == White) {
						op[me] += pawns_on_bishop_colour_opening[pawn_info->wsp[me]];
						eg[me] += pawns_on_bishop_colour_endgame[pawn_info->wsp[me]];
					} else {
						if (me == White) {
							op[me] += pawns_on_bishop_colour_opening[(board->number[WhitePawn12] - pawn_info->wsp[me])];
							eg[me] += pawns_on_bishop_colour_endgame[(board->number[WhitePawn12] - pawn_info->wsp[me])];
						} else {
							op[me] += pawns_on_bishop_colour_opening[(board->number[BlackPawn12] - pawn_info->wsp[me])];
							eg[me] += pawns_on_bishop_colour_endgame[(board->number[BlackPawn12] - pawn_info->wsp[me])];
						}
					}

					break;
				}

				case Rook64: {

					// mobility
					S32 mob = 0;
					S16 capture;

					for (U8 i = 0; i < rook_moves.size(); ++i) {
						for (S16 to = *from + rook_moves[i]; capture = board->square[to], THROUGH(capture);
						     to += rook_moves[i])
							mob += MobMove;
						mob += unit[capture];
					}

					op[me] += rook_mob_open[mob];
					eg[me] += rook_mob_end[mob];

					op[me] += (rook_tropism_opening * 8)
						- (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * rook_tropism_opening));
					eg[me] += (rook_tropism_endgame * 8)
						- (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * rook_tropism_endgame));

					// open file
					op[me] -= RookOpenFileOpening / 2;
					eg[me] -= RookOpenFileEndgame / 2;

					if (board->pawn_file[me][piece_file] == 0) { // no friendly pawn

						op[me] += RookSemiOpenFileOpening;
						eg[me] += RookSemiOpenFileEndgame;

						if (board->pawn_file[opp][piece_file] == 0) { // no enemy pawn
							op[me] += RookOpenFileOpening - RookSemiOpenFileOpening;
							eg[me] += RookOpenFileEndgame - RookSemiOpenFileEndgame;
						} else {

							const S32 BadPawnFile = 1 << (piece_file - FileA); // HACK: see BadPawnFileA and FileA
							if ((pawn_info->badpawns[opp] & BadPawnFile) != 0) {
								op[me] += RookOnBadPawnFileOpening;
								eg[me] += RookOnBadPawnFileEndgame;
							}
						}

						if ((mat_info->cflags[opp] & MatKingFlag) != 0) {

							S16 delta = abs(piece_file - king_file); // file distance

							if (delta <= 1) {
								op[me] += RookSemiKingFileOpening;
								if (delta == 0) op[me] += RookKingFileOpening - RookSemiKingFileOpening;
							}
						}
					}

					// 7th rank

					if (PAWN_RANK(*from, me) == Rank7) {
						if ((pawn_info->flags[opp] & BackRankFlag) != 0 // opponent pawn on 7th rank
							|| PAWN_RANK(KING_POS(board, opp), me) == Rank8) {
							op[me] += Rook7thOpening;
							eg[me] += Rook7thEndgame;
						}
					}

					break;
				}

				case Queen64: {

					// mobility
					S32 mob = 0;
					S16 capture;

					for (U8 i = 0; i < queen_moves.size(); ++i) {
						for (S16 to = *from + queen_moves[i]; capture = board->square[to], THROUGH(capture);
						     to += queen_moves[i])
							mob += MobMove;
						mob += unit[capture];
					}

					op[me] += queen_mob_open[mob];
					eg[me] += queen_mob_end[mob];

					op[me] += (queen_tropism_opening * 8)
						- (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * queen_tropism_opening));
					eg[me] += (queen_tropism_endgame * 8)
						- (((abs(king_file - piece_file) + abs(king_rank - piece_rank)) * queen_tropism_endgame));

					// 7th rank

					if (PAWN_RANK(*from, me) == Rank7) {
						if ((pawn_info->flags[opp] & BackRankFlag) != 0 // opponent pawn on 7th rank
							|| PAWN_RANK(KING_POS(board, opp), me) == Rank8) {
							op[me] += Queen7thOpening;
							eg[me] += Queen7thEndgame;
						}
					}

					break;
				}
			}
		}
	}

	// update
	*opening += ((op[White] - op[Black]) * PieceActivityWeight) / 256;
	*endgame += ((eg[White] - eg[Black]) * PieceActivityWeight) / 256;
}

// eval_king()

static void eval_king(const board_t *board, const material_info_t *mat_info, S32 *opening, S32 *endgame) {

	ASSERT(board != NULL);
	ASSERT(mat_info != NULL);
	ASSERT(opening != NULL);
	ASSERT(endgame != NULL);

	// init
	S32 op[ColourNb], eg[ColourNb];
	op[0] = op[1] = eg[0] = eg[1] = 0;

	// white pawn shelter
	if ((mat_info->cflags[White] & MatKingFlag) != 0) {

		/* Thomas simple pattern king safety */
		king_is_safe[White] = false;

		if (KingSafety) {
			if (board->square[G1] == WK || board->square[H1] == WK) {
				if (board->square[G2] == WP && (board->square[H2] == WP || board->square[H3] == WP)) {
					king_is_safe[White] = true;
				} else if (board->square[F2] == WP && board->square[G3] == WP && board->square[H2] == WP) {
					king_is_safe[White] = true;
				}
			} else if (board->square[B1] == WK || board->square[A1] == WK) {
				if (board->square[B2] == WP && (board->square[A2] == WP || board->square[A3] == WP)) {
					king_is_safe[White] = true;
				} else if (board->square[C2] == WP && board->square[B3] == WP && board->square[A2] == WP) {
					king_is_safe[White] = true;
				}
			}
		}

		if (!king_is_safe[White]) {
			const S8 me = White;

			// king
			const S32 penalty_1 = shelter_square(board, KING_POS(board, me), me);

			// castling
			S32 penalty_2 = penalty_1;

			if ((board->flags & FlagsWhiteKingCastle) != 0) {
				const S32 tmp = shelter_square(board, G1, me);
				if (tmp < penalty_2) penalty_2 = tmp;
			}

			if ((board->flags & FlagsWhiteQueenCastle) != 0) {
				const S32 tmp = shelter_square(board, B1, me);
				if (tmp < penalty_2) penalty_2 = tmp;
			}

			ASSERT(penalty_2 >= 0 && penalty_2 <= penalty_1);

			// penalty
			const S32 penalty = (penalty_1 + penalty_2) / 2;
			ASSERT(penalty >= 0);

			op[me] -= (penalty * ShelterOpening) / 256;
		}
	}

	// black pawn shelter
	if ((mat_info->cflags[Black] & MatKingFlag) != 0) {

		king_is_safe[Black] = false;

		if (KingSafety) {
			if (board->square[G8] == BK || board->square[H8] == BK) {
				if (board->square[G7] == BP && (board->square[H7] == BP || board->square[H6] == BP)) {
					king_is_safe[Black] = true;
				} else if (board->square[F7] == BP && board->square[G6] == BP && board->square[H7] == BP) {
					king_is_safe[Black] = true;
				}
			} else if (board->square[B8] == BK || board->square[A8] == BK) {
				if (board->square[B7] == BP && (board->square[A7] == BP || board->square[A6] == BP)) {
					king_is_safe[Black] = true;
				} else if (board->square[C7] == BP && board->square[B6] == BP && board->square[A7] == BP) {
					king_is_safe[Black] = true;
				}
			}
		}

		if (king_is_safe[Black] == false) {

			const S8 me = Black;

			// king
			const S16 penalty_1 = shelter_square(board, KING_POS(board, me), me);

			// castling
			S16 penalty_2 = penalty_1;

			if ((board->flags & FlagsBlackKingCastle) != 0) {
				const S16 tmp = shelter_square(board, G8, me);
				if (tmp < penalty_2) penalty_2 = tmp;
			}

			if ((board->flags & FlagsBlackQueenCastle) != 0) {
				const S16 tmp = shelter_square(board, B8, me);
				if (tmp < penalty_2) penalty_2 = tmp;
			}

			ASSERT(penalty_2 >= 0 && penalty_2 <= penalty_1);

			// penalty
			const S16 penalty = (penalty_1 + penalty_2) / 2;
			ASSERT(penalty >= 0);

			op[me] -= (penalty * ShelterOpening) / 256;
		}
	}

	// king attacks

	for (S8 colour = 0; colour < ColourNb; ++colour) {

		if ((mat_info->cflags[colour] & MatKingFlag) != 0) {

			const S8 me = colour, opp = COLOUR_OPP(me);
			const S16 king = KING_POS(board, me);
			//const S8 king_file = SQUARE_FILE(king), king_rank = SQUARE_RANK(king);

			// piece attacks
			S16 attack_tot = 0, piece_nb = 0;
			for (auto from = board->piece[opp].begin() + 1; from != board->piece[opp].end(); ++from) { // HACK: no king

				const S16 piece = board->square[*from];

				if (piece_attack_king(board, piece, *from, king)) {
					++piece_nb;
					attack_tot += KingAttackUnit[piece];
				}
/*				 else{
					 if ((abs(king_file-SQUARE_FILE(*from)) + abs(king_rank-SQUARE_RANK(*from))) <= 4)  {
						++piece_nb;
						attack_tot += KingAttackUnit[piece];
					}
				} */
			}

			// scoring
			ASSERT(piece_nb >= 0 && piece_nb < 16);
			op[colour] -= (attack_tot * KingAttackOpening * KingAttackWeight[piece_nb]) / 256;

		}
	}

	// update
	*opening += (op[White] - op[Black]);
	*endgame += (eg[White] - eg[Black]);
}

// eval_passer()

static void eval_passer(const board_t *board, const pawn_info_t *pawn_info, S32 *opening, S32 *endgame) {

	ASSERT(board != NULL);
	ASSERT(pawn_info != NULL);
	ASSERT(opening != NULL);
	ASSERT(endgame != NULL);

	// init
	S32 op[ColourNb], eg[ColourNb];
	op[0] = op[1] = eg[0] = eg[1] = 0;

	//S8 white_passed_nb = 0, black_passed_nb = 0;

	// passed pawns
	for (S8 colour = 0; colour < ColourNb; ++colour) {

		const S8 att = colour, def = COLOUR_OPP(att);

		for (S32 bits = pawn_info->passed_bits[att]; bits != 0; bits &= bits - 1) {

			const S8 file = BIT_FIRST(bits);
			ASSERT(file >= FileA && file <= FileH);

			const S8 rank = BIT_LAST(board->pawn_file[att][file]);
			ASSERT(rank >= Rank2 && rank <= Rank7);

			S16 sq = SQUARE_MAKE(file, rank);
			if (COLOUR_IS_BLACK(att)) sq = SQUARE_RANK_MIRROR(sq);

			ASSERT(PIECE_IS_PAWN(board->square[sq]));
			ASSERT(COLOUR_IS(board->square[sq], att));


			// opening scoring
			op[att] += quad(PassedOpeningMin, PassedOpeningMax, rank);

			// endgame scoring init
			S16 min = PassedEndgameMin, max = PassedEndgameMax, delta = max - min;
			ASSERT(delta > 0);

			// "dangerous" bonus
			if (board->piece[def].size() <= 1 // defender has no piece
				&& (unstoppable_passer(board, sq, att) || king_passer(board, sq, att))) {
				delta += UnstoppablePasser;
			} else if (free_passer(board, sq, att)) {
				delta += FreePasser;
			}

			// king-distance bonus
			delta -= pawn_att_dist(sq, KING_POS(board, att), att) * AttackerDistance;
			delta += pawn_def_dist(sq, KING_POS(board, def), att) * DefenderDistance;

			// endgame scoring
			eg[att] += min;
			if (delta > 0) eg[att] += quad(0, delta, rank);
		}
	}

	// update
	*opening += ((op[White] - op[Black]) * PassedPawnWeight) / 256;
	*endgame += ((eg[White] - eg[Black]) * PassedPawnWeight) / 256;
}

// eval_pattern()

static void eval_pattern(const board_t *board, S32 *opening, S32 *endgame) {

	ASSERT(board != NULL);
	ASSERT(opening != NULL);
	ASSERT(endgame != NULL);

	// trapped bishop (7th rank)

	if ((board->square[A7] == WB && board->square[B6] == BP) || (board->square[B8] == WB && board->square[C7] == BP)) {
		*opening -= TrappedBishop;
		*endgame -= TrappedBishop;
	}

	if ((board->square[H7] == WB && board->square[G6] == BP) || (board->square[G8] == WB && board->square[F7] == BP)) {
		*opening -= TrappedBishop;
		*endgame -= TrappedBishop;
	}

	if ((board->square[A2] == BB && board->square[B3] == WP) || (board->square[B1] == BB && board->square[C2] == WP)) {
		*opening += TrappedBishop;
		*endgame += TrappedBishop;
	}

	if ((board->square[H2] == BB && board->square[G3] == WP) || (board->square[G1] == BB && board->square[F2] == WP)) {
		*opening += TrappedBishop;
		*endgame += TrappedBishop;
	}

	// trapped bishop (6th rank)

	if (board->square[A6] == WB && board->square[B5] == BP) {
		*opening -= TrappedBishop / 2;
		*endgame -= TrappedBishop / 2;
	}

	if (board->square[H6] == WB && board->square[G5] == BP) {
		*opening -= TrappedBishop / 2;
		*endgame -= TrappedBishop / 2;
	}

	if (board->square[A3] == BB && board->square[B4] == WP) {
		*opening += TrappedBishop / 2;
		*endgame += TrappedBishop / 2;
	}

	if (board->square[H3] == BB && board->square[G4] == WP) {
		*opening += TrappedBishop / 2;
		*endgame += TrappedBishop / 2;
	}

	// blocked bishop

	if (board->square[D2] == WP && board->square[D3] != Empty && board->square[C1] == WB) {
		*opening -= BlockedBishop;
	}

	if (board->square[E2] == WP && board->square[E3] != Empty && board->square[F1] == WB) {
		*opening -= BlockedBishop;
	}

	if (board->square[D7] == BP && board->square[D6] != Empty && board->square[C8] == BB) {
		*opening += BlockedBishop;
	}

	if (board->square[E7] == BP && board->square[E6] != Empty && board->square[F8] == BB) {
		*opening += BlockedBishop;
	}

	// blocked rook

	if ((board->square[C1] == WK || board->square[B1] == WK)
		&& (board->square[A1] == WR || board->square[A2] == WR || board->square[B1] == WR)) {
		*opening -= BlockedRook;
	}

	if ((board->square[F1] == WK || board->square[G1] == WK)
		&& (board->square[H1] == WR || board->square[H2] == WR || board->square[G1] == WR)) {
		*opening -= BlockedRook;
	}

	if ((board->square[C8] == BK || board->square[B8] == BK)
		&& (board->square[A8] == BR || board->square[A7] == BR || board->square[B8] == BR)) {
		*opening += BlockedRook;
	}

	if ((board->square[F8] == BK || board->square[G8] == BK)
		&& (board->square[H8] == BR || board->square[H7] == BR || board->square[G8] == BR)) {
		*opening += BlockedRook;
	}

	// White center pawn blocked
	if (board->square[E2] == BP && board->square[E3] != Empty) {
		*opening -= BlockedCenterPawn;
	}
	if (board->square[D2] == BP && board->square[D3] != Empty) {
		*opening -= BlockedCenterPawn;
	}

	// Black center pawn blocked
	if (board->square[E7] == BP && board->square[E6] != Empty) {
		*opening += BlockedCenterPawn;
	}
	if (board->square[D7] == BP && board->square[D6] != Empty) {
		*opening += BlockedCenterPawn;
	}

}

// unstoppable_passer()

static bool unstoppable_passer(const board_t *board, S32 pawn, S8 colour) {

	ASSERT(board != NULL);
	ASSERT(SQUARE_IS_OK(pawn));
	ASSERT(COLOUR_IS_OK(colour));

	const S8 me = colour, opp = COLOUR_OPP(me);

	const S8 file = SQUARE_FILE(pawn);
	const S32 king = KING_POS(board, opp);
	S8 rank = PAWN_RANK(pawn, me);
	// clear promotion path?

	for (auto sq = board->piece[me].begin(); sq != board->piece[me].end(); ++sq) {
		if (SQUARE_FILE(*sq) == file && PAWN_RANK(*sq, me) > rank) {
			return false; // "friendly" blocker
		}
	}

	// init

	if (rank == Rank2) {
		pawn += PAWN_MOVE_INC(me);
		++rank;
		ASSERT(rank == PAWN_RANK(pawn, me));
	}

	ASSERT(rank >= Rank3 && rank <= Rank7);

	const S16 prom = PAWN_PROMOTE(pawn, me);
	S16 dist = DISTANCE(pawn, prom);
	ASSERT(dist == Rank8 - rank);

	if (board->turn == opp) ++dist;

	if (DISTANCE(king, prom) > dist) return true; // not in the square

	return false;
}

// king_passer()

static bool king_passer(const board_t *board, S32 pawn, S8 colour) {

	ASSERT(board != NULL);
	ASSERT(SQUARE_IS_OK(pawn));
	ASSERT(COLOUR_IS_OK(colour));

	const S16 king = KING_POS(board, colour), file = SQUARE_FILE(pawn), prom = PAWN_PROMOTE(pawn, colour);

	if (DISTANCE(king, prom) <= 1 && DISTANCE(king, pawn) <= 1
		&& (SQUARE_FILE(king) != file || (file != FileA && file != FileH))) {
		return true;
	}

	return false;
}

// free_passer()

static bool free_passer(const board_t *board, S32 pawn, S8 colour) {

	ASSERT(board != NULL);
	ASSERT(SQUARE_IS_OK(pawn));
	ASSERT(COLOUR_IS_OK(colour));

	const S16 inc = PAWN_MOVE_INC(colour), sq = pawn + inc;
	ASSERT(SQUARE_IS_OK(sq));

	if (board->square[sq] != Empty) return false;

	const U16 move = MOVE_MAKE(pawn, sq);
	if (see_move(move, board) < 0) return false;

	return true;
}

// pawn_att_dist()

static S32 pawn_att_dist(S32 pawn, S32 king, S8 colour) {

	ASSERT(SQUARE_IS_OK(pawn));
	ASSERT(SQUARE_IS_OK(king));
	ASSERT(COLOUR_IS_OK(colour));

	const S8 inc = PAWN_MOVE_INC(colour);

	const S16 target = pawn + inc;

	return DISTANCE(king, target);
}

// pawn_def_dist()

static S32 pawn_def_dist(S32 pawn, S32 king, S8 colour) {

	ASSERT(SQUARE_IS_OK(pawn));
	ASSERT(SQUARE_IS_OK(king));
	ASSERT(COLOUR_IS_OK(colour));

	const S8 inc = PAWN_MOVE_INC(colour);
	const S16 target = pawn + inc;

	return DISTANCE(king, target);
}

// draw_init_list()

static void draw_init_list(S32 list[], const board_t *board, S32 pawn_colour) {

	ASSERT(list != NULL);
	ASSERT(board != NULL);
	ASSERT(COLOUR_IS_OK(pawn_colour));

	// init
	const S8 att = pawn_colour, def = COLOUR_OPP(att);

	ASSERT(board->pawn[att].size() == 1);
	ASSERT(board->pawn[def].size() == 0);

	S32 pos = 0;

	// att
	for (auto sq = board->piece[att].begin(); sq != board->piece[att].end(); ++sq) {
		list[pos++] = *sq;
	}

	for (auto sq = board->pawn[att].begin(); sq != board->pawn[att].end(); ++sq) {
		list[pos++] = *sq;
	}

	// def
	for (auto sq = board->piece[def].begin(); sq != board->piece[def].end(); ++sq) {
		list[pos++] = *sq;
	}

	for (auto sq = board->pawn[def].begin(); sq != board->pawn[def].end(); ++sq) {
		list[pos++] = *sq;
	}

	// end marker

	ASSERT(pos == board->piece_nb);

	list[pos] = SquareNone;

	// file flip?

	const S16 pawn = board->pawn[att][0];

	if (SQUARE_FILE(pawn) >= FileE) {
		for (S16 i = 0; i < pos; ++i) {
			list[i] = SQUARE_FILE_MIRROR(list[i]);
		}
	}

	// rank flip?

	if (COLOUR_IS_BLACK(pawn_colour)) {
		for (S16 i = 0; i < pos; ++i) {
			list[i] = SQUARE_RANK_MIRROR(list[i]);
		}
	}
}

// draw_krpkr()

static bool draw_krpkr(const S32 *list) {

	ASSERT(list != nullptr);

	// load
	S16 wk, wr, wp, bk, br;
	wk = *list++;
	ASSERT(SQUARE_IS_OK(wk));

	wr = *list++;
	ASSERT(SQUARE_IS_OK(wr));

	wp = *list++;
	ASSERT(SQUARE_IS_OK(wp));
	ASSERT(SQUARE_FILE(wp) <= FileD);

	bk = *list++;
	ASSERT(SQUARE_IS_OK(bk));

	br = *list++;
	ASSERT(SQUARE_IS_OK(br));

	ASSERT(*list == SquareNone);

	// test

	const S8 wp_file = SQUARE_FILE(wp), wp_rank = SQUARE_RANK(wp);
	const S8 bk_file = SQUARE_FILE(bk), bk_rank = SQUARE_RANK(bk);
	const S8 br_file = SQUARE_FILE(br), br_rank = SQUARE_RANK(br);

	const S8 prom = PAWN_PROMOTE(wp, White);

	if (false) {
	} else if (bk == prom) {
		// TODO: rook near Rank1 if wp_rank == Rank6?
		if (br_file > wp_file) return true;
	} else if (bk_file == wp_file && bk_rank > wp_rank) {
		return true;
	} else if (wr == prom && wp_rank == Rank7 && (bk == G7 || bk == H7) && br_file == wp_file) {
		if (br_rank <= Rank3) {
			if (DISTANCE(wk, wp) > 1) return true;
		} else { // br_rank >= Rank4
			if (DISTANCE(wk, wp) > 2) return true;
		}
	}

	return false;
}

// draw_kbpkb()

static bool draw_kbpkb(const S32 *list) {

	ASSERT(list != nullptr);

	// load
	S16 wk, wb, wp, bk, bb;
	wk = *list++;
	ASSERT(SQUARE_IS_OK(wk));

	wb = *list++;
	ASSERT(SQUARE_IS_OK(wb));

	wp = *list++;
	ASSERT(SQUARE_IS_OK(wp));
	ASSERT(SQUARE_FILE(wp) <= FileD);

	bk = *list++;
	ASSERT(SQUARE_IS_OK(bk));

	bb = *list++;
	ASSERT(SQUARE_IS_OK(bb));

	ASSERT(*list == SquareNone);

	// opposit colour?
	if (SQUARE_COLOUR(wb) == SQUARE_COLOUR(bb)) return false; // TODO

	// blocked pawn?
	const S8 inc = PAWN_MOVE_INC(White);
	const S16 end = PAWN_PROMOTE(wp, White) + inc;

	for (S16 to = wp + inc; to != end; to += inc) {

		ASSERT(SQUARE_IS_OK(to));

		if (to == bb) return true; // direct blockade

		const S16 delta = to - bb;
		ASSERT(delta_is_ok(delta));

		if (PSEUDO_ATTACK(BB, delta)) {

			const S8 inc_2 = DELTA_INC_ALL(delta);
			ASSERT(inc_2 != IncNone);

			S16 sq = bb;
			do {
				sq += inc_2;
				ASSERT(SQUARE_IS_OK(sq));
				ASSERT(sq != wk);
				ASSERT(sq != wb);
				ASSERT(sq != wp);
				ASSERT(sq != bb);
				if (sq == to) return true; // indirect blockade
			} while (sq != bk);
		}
	}

	return false;
}

// shelter_square()

static S32 shelter_square(const board_t *board, S32 square, S8 colour) {

	ASSERT(board != NULL);
	ASSERT(SQUARE_IS_OK(square));
	ASSERT(COLOUR_IS_OK(colour));

	const S8 file = SQUARE_FILE(square), rank = PAWN_RANK(square, colour);

	S32 penalty = (shelter_file(board, file, rank, colour) * 2);
	if (file != FileA) penalty += (shelter_file(board, file - 1, rank, colour));
	if (file != FileH) penalty += (shelter_file(board, file + 1, rank, colour));

	if (penalty == 0) penalty = 11; // weak back rank

	penalty += storm_file(board, file, colour);
	if (file != FileA) penalty += storm_file(board, file - 1, colour);
	if (file != FileH) penalty += storm_file(board, file + 1, colour);

	return penalty;
}

// shelter_file()

static S8 shelter_file(const board_t *board, S32 file, S32 rank, S8 colour) {

	ASSERT(board != NULL);
	ASSERT(file >= FileA && file <= FileH);
	ASSERT(rank >= Rank1 && rank <= Rank8);
	ASSERT(COLOUR_IS_OK(colour));

	S8 dist = BIT_FIRST(board->pawn_file[colour][file] & BitGE[rank]);
	ASSERT(dist >= Rank2 && dist <= Rank8);

	dist = Rank8 - dist;
	ASSERT(dist >= 0 && dist <= 6);

	S8 penalty = 36 - dist * dist;
	ASSERT(penalty >= 0 && penalty <= 36);

	return penalty;
}

// storm_file()

static S16 storm_file(const board_t *board, S32 file, S8 colour) {

	ASSERT(board != NULL);
	ASSERT(file >= FileA && file <= FileH);
	ASSERT(COLOUR_IS_OK(colour));

	S8 dist = BIT_LAST(board->pawn_file[COLOUR_OPP(colour)][file]);
	ASSERT(dist >= Rank1 && dist <= Rank7);

	S16 penalty = 0;

	switch (dist) {
		case Rank4: penalty = StormOpening * 1;
	        break;
		case Rank5: penalty = StormOpening * 3;
	        break;
		case Rank6: penalty = StormOpening * 6;
	        break;
	}

	return penalty;
}

// bishop_can_attack()

static bool bishop_can_attack(const board_t *board, S32 to, S8 colour) {

	ASSERT(board != NULL);
	ASSERT(SQUARE_IS_OK(to));
	ASSERT(COLOUR_IS_OK(colour));

	for (auto from = board->piece[colour].begin() + 1; from != board->piece[colour].end(); ++from) { // HACK: no king
		S16 piece = board->square[*from];
		if (PIECE_IS_BISHOP(piece) && SQUARE_COLOUR(*from) == SQUARE_COLOUR(to))
			return true;
	}
	return false;
}

// end of eval.cpp
