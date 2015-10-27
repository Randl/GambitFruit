//
// Created by Evgenii on 29.08.2015.
//

#include <cmath>
#include <fstream>
#include <iostream>
#include "texel_optimizer.h"

//TODO: add epd generator
double epd_to_fen(std::string &epd) {
	uint_fast64_t pos = 0;
	for (int i = 0; i < 4; ++i)
		pos = epd.find_first_of(' ', pos + 1);
	uint_fast64_t comm = epd.find("c9 \"") + 4;
	std::string result = epd.substr(comm, epd.find_first_of('"', comm) - comm);
	epd = epd.substr(0, pos) + " 0 1";
	if (result == "1-0")
		return 1;
	if (result == "1/2-1/2")
		return 0.5;
	return 0;
}

std::vector<std::pair<std::string, double>> load_epds(std::string filename) {
	std::ifstream e(filename);
	std::vector<std::pair<std::string, double>> positions;
	std::string line;
	while (getline(e, line)) {
		double d = epd_to_fen(line);
		positions.push_back(std::make_pair(line, d));
	}
	e.close();
	return positions;
}

//TODO: Calculate K at start
/*
double (std::vector<std::pair<std::string, double>> positions, double K, int_fast16_t limit, std::vector<int_fast16_t> params) {
	double E = 0.0;
	int count = 0;

	engine_init();
	engine_set_options(options, point);

	for (auto fen: positions) {
		double R = fen.second;

		int_fast32_t eval = call_q_search(fen.first);
		E += std::pow(R - 1.0 / (1 + std::pow(10.0, -vK(0) * eval / 400.0)), 2);
		++count;
	}

	E = count != 0 ? E / count : 0;
	return E;
}
*/



double average_error(std::vector<std::pair<std::string, double>> positions,
                     std::vector<std::string> options,
                     double K,
                     int_fast16_t limit,
                     std::vector<int_fast16_t> params) {
//TODO: multithreading; Find all globals to make them private?
	double E = 0.0;
	int count = 0;

	engine_init();
	engine_set_options(options, params);

	//#pragma omp parallel for private()
	for (size_t i = 0; i < positions.size(); ++i) {
		double R = positions[i].second;
		int_fast32_t eval = call_q_search(positions[i].first);

		if (i % 100000 == 0 && i != 0)
			std::cout << "Evaluated " << i << " positions out of " << positions.size() << ". " << count
				<< " positions count." << std::endl;
		if (eval >= limit || eval <= -limit)
			continue;
		E += std::pow(R - 1.0 / (1.0 + std::pow(10.0, -K * eval / 400.0)), 2.0);
		++count;
	}
	E /= count;
	return E;
}