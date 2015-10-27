//
// Created by Evgenii on 29.08.2015.
//

#ifndef TMO_ENGINE_CONNECTOR_H
#define TMO_ENGINE_CONNECTOR_H

#include <string>
#include <vector>

int_fast32_t call_q_search(std::string fen);
void engine_init();
void engine_set_options(std::vector<std::string> options, std::vector<int_fast16_t> params);
#endif //TMO_ENGINE_CONNECTOR_H
