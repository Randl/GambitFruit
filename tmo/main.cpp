#include <iostream>
#include <functional>
#include "hill_climbing.h"
#include "texel_optimizer.h"


int main() {
	//TODO: ini
	std::vector<std::string> options
		{"Opening Pawn Value", "Opening Knight Value", "Opening Bishop Value", "Opening Rook Value",
		 "Opening Queen Value", "Endgame Pawn Value", "Endgame Knight Value", "Endgame Bishop Value",
		 "Endgame Rook Value", "Endgame Queen Value", "Bishop Pair Opening", "Bishop Pair Endgame",
		 "Queen Knight combo", "Rook Bishop combo", "Bad Trade Value", "PawnAmountBonusOpening0",
		 "PawnAmountBonusOpening1", "PawnAmountBonusOpening2", "PawnAmountBonusOpening3", "PawnAmountBonusOpening4",
		 "PawnAmountBonusOpening5", "PawnAmountBonusOpening6", "PawnAmountBonusOpening7", "PawnAmountBonusOpening8",
		 "PawnAmountBonusEndgame0", "PawnAmountBonusEndgame1", "PawnAmountBonusEndgame2", "PawnAmountBonusEndgame3",
		 "PawnAmountBonusEndgame4", "PawnAmountBonusEndgame5", "PawnAmountBonusEndgame6", "PawnAmountBonusEndgame7",
		 "PawnAmountBonusEndgame8"};
	std::vector<int_fast16_t> v
		{100, 300, 300, 500, 1000, 100, 300, 300, 500, 1000, 75, 75, 50, 35, 15, -10, -9, -6, 0, 5, 5, 5, 2, -4, -55,
		 -20, -2, 0, 10, 8, 5, 3, -8};
	std::vector<int_fast16_t> min
		{50, 200, 200, 350, 700, 50, 200, 200, 350, 700, 30, 30, 15, 10, 5, -30, -30, -30, -30, -30, -30, -30, -30, -30,
		 -90, -50, -30, -30, -30, -30, -30, -30, -30,};
	std::vector<int_fast16_t> max
		{200, 500, 500, 800, 1000, 200, 500, 500, 800, 1000, 150, 150, 90, 60, 60, 20, 20, 20, 20, 20, 20, 20, 20, 20,
		 20, 30, 20, 20, 20, 20, 20, 20, 20,};

	double K = 0.14;
	int_fast16_t limit = 500;
	std::vector<std::pair<std::string, double>> large = load_epds("large.epds");
	std::vector<std::pair<std::string, double>> small = load_epds("small.epds");
	std::function<double(std::vector<int_fast16_t>)>
		est = [=](std::vector<int_fast16_t> params) { return average_error(small, options, K, limit, params); };
	std::function<double(std::vector<int_fast16_t>)>
		prec = [=](std::vector<int_fast16_t> params) { return average_error(large, options, K, limit, params); };
	solver(est, prec, v, min, max, 100, 200, 10, 10);
	return 0;
}