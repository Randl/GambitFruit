//
// Created by Evgenii on 27.10.2015.
//

#ifndef TMO_HILL_CLIMBING_H
#define TMO_HILL_CLIMBING_H

#include <vector>

std::vector<int_fast16_t> solver(std::function<double(std::vector<int_fast16_t>)> estimated,
                                 std::function<double(std::vector<int_fast16_t>)> precision,
                                 std::vector<int_fast16_t> starting_point,
                                 std::vector<int_fast16_t> min,
                                 std::vector<int_fast16_t> max,
                                 int_fast16_t pool_size,
                                 int_fast16_t seed_size,
                                 int_fast16_t num_candidates,
                                 int_fast16_t tournament_size);
#endif //TMO_HILL_CLIMBING_H
