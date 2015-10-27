#include <iostream>
#include <functional>
#include "hill_climbing.h"
#include "texel_optimizer.h"
#include "3rdParty/SimpleIni.h"

void load_ini(std::string filename,
              std::vector<std::string> &options,
              std::vector<int_fast16_t> &v,
              std::vector<int_fast16_t> &min,
              std::vector<int_fast16_t> &max) {
	// LOADING DATA

	// load from a data file
	CSimpleIniA ini(false, false, false);
	SI_Error rc = ini.LoadFile(filename.c_str());
	if (rc < 0) return;

	// GETTING SECTIONS AND KEYS

	// get all sections
	CSimpleIniA::TNamesDepend sections;
	ini.GetAllSections(sections);

	// GETTING VALUES
	for (auto section : sections) {
		int_fast16_t val = std::atoi(ini.GetValue(section.pItem, "value", NULL)),
			mn = std::atoi(ini.GetValue(section.pItem, "min", NULL)),
			mx = std::atoi(ini.GetValue(section.pItem, "max", NULL));
		v.push_back(val);
		min.push_back(mn);
		max.push_back(mx);
		options.push_back(section.pItem);
	}
}

int main() {
	//TODO: ini options for optimization

	std::vector<std::string> options;
	std::vector<int_fast16_t> v;
	std::vector<int_fast16_t> min;
	std::vector<int_fast16_t> max;

	load_ini("options.ini", options, v, min, max);

	for (int i = 0; i < options.size(); ++i)
		std::cout << options[i] << " " << v[i] << " " << min[i] << " " << max[i] << std::endl;

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