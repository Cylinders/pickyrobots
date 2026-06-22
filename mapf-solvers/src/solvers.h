#ifndef MAPF_SOLVER_ADAPTERS_H
#define MAPF_SOLVER_ADAPTERS_H

#include <string>
#include "mapf_common/solution.h"

// Solves using the standard CBS algorithm
mapf::Solution cbs_solve(const std::string& mapFilePath, const std::string& scenFilePath, const int n);

// Solves using the CBSH (CBS with Heuristics) algorithm
mapf::Solution cbsh_solve(const std::string& mapFilePath, const std::string& scenFilePath, const int n);

// Solves using the BCP algorithm
mapf::Solution bcp_solve(const int t, const std::string& input);

#endif // MAPF_SOLVER_ADAPTERS_H
