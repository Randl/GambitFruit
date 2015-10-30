//
// Created by Evgenii on 27.10.2015.
//

#ifndef TMO_HILL_CLIMBING_H
#define TMO_HILL_CLIMBING_H

#include <vector>
#include "../GambitFruit/util.h"

double calculateK(std::function<double(double)> f, double lowest, double highest, double tol = 1e-5);
std::vector<S16> solver(std::function<double(std::vector<S16>)> estimated,
                        std::function<double(std::vector<S16>)> precision,
                        std::vector<S16> starting_point,
                        std::vector<S16> min,
                        std::vector<S16> max,
                        S16 pool_size,
                        S16 seed_size,
                        S16 num_candidates,
                        S16 tournament_size);
#endif //TMO_HILL_CLIMBING_H
