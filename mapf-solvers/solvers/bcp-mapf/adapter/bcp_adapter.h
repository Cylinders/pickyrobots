#pragma once
#include <string>

#include "mapf_common/solution.h"
namespace mapf_solvers::bcp {
    mapf::Solution bcp_solve(int t, const std::string& input);
}
