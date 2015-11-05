#include <string>
#include <array>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
extern "C" {
#include "3rdParty/pgn-extract/pgn-extract.h"
}
void replaceAll(std::string &s, const std::string &search, const std::string &replace) {
	for (size_t pos = 0; ; pos += replace.length()) {
		// Locate the substring to replace
		pos = s.find(search, pos);
		if (pos == std::string::npos) break;
		// Replace by erasing and inserting
		s.erase(pos, search.length());
		s.insert(pos, replace);
	}
}

std::array<std::string, 3> pgn_process(std::string in) {
	const char *ar[3] = {"tmp1", "tmp2", "tmp3"};
	std::array<std::string, 3> res = {"wwin", "bwin", "draw"};
	divide(in.c_str(), ar);
	for (int i = 0; i < 3; ++i)
		extract(ar[i], res[i].c_str());
	return res;
};

std::array<std::vector<std::string>, 3> to_epds(std::array<std::string, 3> res) {

	std::array<std::vector<std::string>, 3> positions;

	for (int i = 0; i < 3; ++i) {
		std::string file = res[i], result = file == "wwin" ? "1-0" : file == "bwin" ? "0-1" : "1/2-1/2";
		std::ifstream t(file);
		std::string str;

		t.seekg(0, std::ios::end);
		str.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

		replaceAll(str, "\n", " ");
		replaceAll(str, "}", "\n");

		positions[i].reserve(std::count(str.begin(), str.end(), '\n'));

		size_t begin = 0, start = str.find("{", 0) + 1, ps = str.find("\n", start);
		while (ps != std::string::npos) {

			positions[i].push_back(std::regex_replace(str.substr(start, ps - start),
			                                          std::regex("^ +| +$|( ) +"),
			                                          "$1"));

			begin = ps + 1;
			start = str.find("{", ps) + 1;
			ps = str.find("\n", ps + 1);
		}

		for (auto &x : positions[i]) {
			size_t pos = x.find(" ", 0);
			for (int j = 0; j < 3; ++j)
				pos = x.find(" ", pos + 1);
			x.erase(x.begin() + pos, x.end());
			x += " c9 \"" + result + "\";";
		}
		positions[i].erase(std::unique(positions[i].begin(), positions[i].end()), positions[i].end());

	}
	return positions;
}

std::vector<std::string> create_epd(std::array<std::vector<std::string>, 3> positions,
                                    size_t max_pos_n,
                                    double draws_part) {
	std::default_random_engine gen;
	std::vector<std::string> all_pos;
	all_pos.reserve(max_pos_n);

	size_t result_num = std::min(positions[0].size(), positions[1].size());
	size_t total_pos = (size_t) std::min(floor(2.0 * result_num / (1.0 - draws_part)),
	                                     floor((double) positions[2].size() / draws_part));
	total_pos = total_pos < max_pos_n ? total_pos : max_pos_n;
	size_t used_result_num = (size_t) (floor((1.0 - draws_part) * total_pos / 2.0)),
		used_draws_num = (size_t) (floor(draws_part * total_pos));
	std::array<size_t, 3> num = {used_result_num, used_result_num, used_draws_num};
	for (int i = 0; i < 3; ++i) {
		double part = (double) positions[i].size() / num[i];
		if (std::round(part) > 2) {
			while (positions[i].size() > num[i]) {
				//leave random position out of
				size_t step = std::round(part);
				for (size_t k = 0; k < positions[i].size() && positions[i].size() > num[i]; k += step) {
					std::uniform_int_distribution<size_t> choose(0, step - 1);
					size_t rem = k + choose(gen);
					for (size_t j = k; j < std::min(k + step, positions[i].size() - 1) && positions[i].size() > num[i];
					     ++j) {
						if (j != rem)
							positions[i].erase(positions[i].begin() + j);
					}
				}
			}
		} else if (std::round(part) > 1) {
			while (positions[i].size() > num[i]) {
				size_t step = std::round((part - 1.0) / part);
				for (size_t k = 0; k < positions[i].size() && positions[i].size() > num[i]; k += step) {
					std::uniform_int_distribution<size_t> choose(0, step - 1);
					size_t rem = k + choose(gen);
					positions[i].erase(positions[i].begin() + rem);
				}
			}
		}

		all_pos.insert(all_pos.end(), positions[i].begin(), positions[i].end());
	}

	std::random_shuffle(all_pos.begin(), all_pos.end());
	return all_pos;
}

int main() {
	std::string in = "in.pgn", out = "out.epds";
	std::array<std::string, 3> res = pgn_process(in);
	std::array<std::vector<std::string>, 3> epds = to_epds(res);
	std::vector<std::string> all_epds = create_epd(epds, 100, 0.5);

	std::ofstream result_out(out);
	for (auto &r : all_epds)
		result_out << r << std::endl;
	return 0;
}
