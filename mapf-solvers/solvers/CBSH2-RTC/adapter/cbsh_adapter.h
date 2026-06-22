#pragma once
#include <string>
#include "mapf_common/solution.h"

namespace mapf_solvers::cbsh {
    mapf::Solution cbs_solve(const std::string &mapFilePath, const std::string &scenFilePath, int n);
    mapf::Solution cbsh_solve(const std::string &mapFilePath, const std::string &scenFilePath, int n);
}
