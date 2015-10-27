//
// Created by Evgenii on 27.10.2015.
//

#include <cstdint>
#include <chrono>
#include <algorithm>
#include <functional>
#include <map>
#include <iostream>

struct CompareSecond {
    bool operator()(const std::pair<std::vector<int_fast16_t>, double> &left,
                    const std::pair<std::vector<int_fast16_t>, double> &right) const {
	    return left.second < right.second;
    }
};

//generates new point with normal distribution, 99.7% of values are in (point * (1-range); point * (1+range)). Minimal sigma is 5
std::vector<int_fast16_t> mutate(std::vector<int_fast16_t> starting_point,
                                 double range,
                                 std::vector<int_fast16_t> min,
                                 std::vector<int_fast16_t> max) {
	int_fast64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::default_random_engine gen(seed);

	std::vector<int_fast16_t> new_point;
	for (size_t i = 0; i < starting_point.size(); ++i) {
		double sigma = std::max(std::abs(range * starting_point[i] / 3.0), 5.0);
		std::normal_distribution<double> d((double) starting_point[i], sigma);
		new_point.push_back((int_fast16_t) std::round(d(gen)));
		new_point[i] = new_point[i] > min[i] ? new_point[i] < max[i] ? new_point[i] : max[i] : min[i];
	}
	return new_point;
}

bool add(std::function<double(std::vector<int_fast16_t>)> estimated,
         std::map<std::vector<int_fast16_t>, double> &solutions,
         std::vector<int_fast16_t> sol,
         int_fast16_t pool_size) {
	double n = estimated(sol);
	//Add value if it's better than current worst value;
	if (solutions.size() < pool_size) {
		solutions[sol] = n;
		return true;
	} else {
		std::pair<std::vector<int_fast16_t>, double>
			max = *max_element(solutions.begin(), solutions.end(), CompareSecond());
		if (max.second > n) {
			solutions.erase(max.first);
			solutions[sol] = n;
			std::cout << "Added point. Value: " << n << std::endl;
			return true;
		}
		return false;
	}

}

std::vector<int_fast16_t> pop(std::map<std::vector<int_fast16_t>, double> &solutions,
                              int_fast16_t tournament_size,
                              int_fast64_t seed) {
	std::default_random_engine gen(seed);

	std::uniform_int_distribution<size_t> step(0, solutions.size() - 1);

	auto it = solutions.begin();
	std::advance(it, step(gen));
	std::vector<int_fast16_t> best = it->first;
	for (int_fast16_t i = 0; i < tournament_size; ++i) {
		it = solutions.begin();
		std::advance(it, step(gen));
		if (solutions[best] > it->second)
			best = it->first;
	}

	std::cout << "Picked point. Value: " << solutions[best] << std::endl;
	return best;
}

std::vector<int_fast16_t> child(std::vector<int_fast16_t> first, std::vector<int_fast16_t> second) {

	int_fast64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::default_random_engine gen(seed);
	std::uniform_int_distribution<int_fast8_t> choose(0, 1);

	std::vector<int_fast16_t> child;
	for (int_fast16_t i = 0; i < first.size(); ++i)
		child.push_back(choose(gen) ? first[i] : second[i]);
	return child;

}
void print(const std::vector<int_fast16_t> v) {

	for (size_t i = 0; i < v.size(); ++i) {
		std::cout << v[i] << " ";
	}
}

std::vector<int_fast16_t> solver(std::function<double(std::vector<int_fast16_t>)> estimated,
                                 std::function<double(std::vector<int_fast16_t>)> precision,
                                 std::vector<int_fast16_t> starting_point,
                                 std::vector<int_fast16_t> min,
                                 std::vector<int_fast16_t> max,
                                 int_fast16_t pool_size,
                                 int_fast16_t seed_size,
                                 int_fast16_t num_candidates,
                                 int_fast16_t tournament_size) {
//TODO: comments
	int_fast64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::default_random_engine gen(seed);


	std::cout << "Start optimization" << std::endl;
#ifndef DEBUG
	freopen("log.txt", "w", stdout);
#endif

	std::cout.precision(std::numeric_limits<double>::max_digits10);
	std::cout << "Start optimization" << std::endl;
	auto begin = std::chrono::high_resolution_clock::now(), search_start = begin;

	double current_value = precision(starting_point);
	std::map<std::vector<int_fast16_t>, double> solutions;
	std::vector<int_fast16_t> current = starting_point;

	double mutation_randomness = 0.25;
	int_fast32_t fails = 0;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	for (int_fast32_t iter = 0; ; ++iter) {
		//Generate starting population
		if (iter == 0 || fails > std::uniform_int_distribution<int_fast32_t>(1, 10000)(gen)) {
			mutation_randomness = 0.25;
			std::cout << "***RESTARTING SEARCH***" << std::endl;
			fails = 0;
			for (int_fast16_t i = 0; i < seed_size; ++i) {
				add(estimated,
				    solutions,
				    mutate(starting_point, mutation_randomness, min, max),
				    pool_size); //TODO: Test parralel random startpoint. Parallel
			}

			std::pair<std::vector<int_fast16_t>, double>
				mx = *max_element(solutions.begin(), solutions.end(), CompareSecond());
			std::pair<std::vector<int_fast16_t>, double>
				mn = *min_element(solutions.begin(), solutions.end(), CompareSecond());
			std::cout << "Generated values in range from " << mn.second << " to " << mx.second << std::endl;
			std::cout << "Minimal value: ";
			print(mn.first);
			std::cout << std::endl << "Maximal value: ";
			print(mx.first);
			std::cout << std::endl << std::endl;
			mutation_randomness = 0.1;
			search_start = std::chrono::high_resolution_clock::now();
		}


		int_fast16_t accepted = 0;
		for (int_fast16_t i = 0; i < 5 * num_candidates && accepted < num_candidates; i++) {
			if (add(estimated, solutions, mutate(current, mutation_randomness, min, max), pool_size))
				++accepted;
		}
		if (accepted < num_candidates)
			mutation_randomness *= 1.5;

		std::pair<std::vector<int_fast16_t>, double>
			mx = *max_element(solutions.begin(), solutions.end(), CompareSecond());
		std::pair<std::vector<int_fast16_t>, double>
			mn = *min_element(solutions.begin(), solutions.end(), CompareSecond());
		std::cout << "Generated values in range from " << mn.second << " to " << mx.second << std::endl;
		std::cout << "Minimal value: ";
		print(mn.first);
		std::cout << std::endl << "Maximal value: ";
		print(mx.first);
		std::cout << std::endl << std::endl;

		std::vector<int_fast16_t>
			sol = child(pop(solutions, tournament_size, seed / 2), pop(solutions, tournament_size, seed / 3));
		double new_value = precision(sol);

		//TODO: output for graphs
		std::cout << std::endl << "***ITERATION " << iter + 1 << " FINISHED***" << std::endl;
		auto end = std::chrono::high_resolution_clock::now();
		std::cout << (double) std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() / 1e9
			<< " seconds elapsed." << std::endl;
		std::cout << "Average of "
			<< (double) std::chrono::duration_cast<std::chrono::nanoseconds>(end - search_start).count() / 1e9
				/ (iter + 1) << " seconds per iteration." << std::endl;

		if (new_value < current_value) {
			fails = 0;
			std::cout << "***NEW SOLUTION FOUND***" << std::endl;
			print(sol);
			std::cout << std::endl << " New value is " << new_value << std::endl;

			std::cout << "Old solution: ";
			print(current);
			std::cout << std::endl << "Old value is " << current_value << std::endl;

			current_value = new_value;
			current = sol;
		} else {
			++fails;
			std::cout << "***NO BETTER SOLUTION FOUND***" << std::endl << "Attempt was: ";
			print(sol);
			std::cout << std::endl << " Its value is " << new_value << std::endl;

			std::cout << "Current solution: ";
			print(current);
			std::cout << std::endl << "Current value is " << current_value << std::endl;
			std::cout << fails << " failed attempts in a row" << std::endl;
		}
		std::cout << std::endl;
	}
#pragma clang diagnostic pop
	return current;
}