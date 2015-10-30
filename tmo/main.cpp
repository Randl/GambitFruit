#include <iostream>
#include <functional>
#include "3rdParty/SimpleIni.h"


#include "hill_climbing.h"
#include "texel_optimizer.h"


void load_ini(std::string filename,
              std::vector<std::string> &options,
              std::vector<S16> &v,
              std::vector<S16> &min,
              std::vector<S16> &max) {
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
		S16 val = std::atoi(ini.GetValue(section.pItem, "value", NULL)),
			mn = std::atoi(ini.GetValue(section.pItem, "min", NULL)),
			mx = std::atoi(ini.GetValue(section.pItem, "max", NULL));
		v.push_back(val);
		min.push_back(mn);
		max.push_back(mx);
		options.push_back(section.pItem);
	}
}

int main() {

	std::cout.precision(std::numeric_limits<double>::max_digits10);

#ifdef NDEBUG
	freopen("log.txt", "w", stdout);
#endif

	std::vector<std::string> options;
	std::vector<S16> v;
	std::vector<S16> min;
	std::vector<S16> max;

	load_ini("options.ini", options, v, min, max);

	for (int i = 0; i < options.size(); ++i)
		std::cout << options[i] << " " << v[i] << " " << min[i] << " " << max[i] << std::endl;

	double K = 0.14;
	S16 limit = 500;
	std::vector<std::pair<std::string, double>> large = load_epds("large.epds");
	std::vector<std::pair<std::string, double>> small = load_epds("small.epds");


	std::function<double(double)> est_k = [=](double Kval) { return average_error(large, options, Kval, limit, v); };
	K = calculateK(est_k, 1e-7, 1.0, 1e-4);
	std::cout << "K value is " << K << std::endl;

	std::function<double(std::vector<S16>)>
		est = [=](std::vector<S16> params) { return average_error(small, options, K, limit, params); };
	std::function<double(std::vector<S16>)>
		prec = [=](std::vector<S16> params) { return average_error(large, options, K, limit, params); };
	solver(est, prec, v, min, max, 100, 200, 10, 10);
	return 0;
}