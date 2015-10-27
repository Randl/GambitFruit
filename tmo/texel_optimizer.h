//
// Created by Evgenii on 29.08.2015.
//

#ifndef TMO_TEXEL_OPTIMIZER_H
#define TMO_TEXEL_OPTIMIZER_H

#include <string>
#include <vector>
#include "engine_connector.h"

std::vector<std::pair<std::string, double>> load_epds(std::string filename);
double average_error(std::vector<std::pair<std::string, double>> positions,
                     std::vector<std::string> options,
                     double K,
                     int_fast16_t limit,
                     std::vector<int_fast16_t> params);

#endif //TMO_TEXEL_OPTIMIZER_H
