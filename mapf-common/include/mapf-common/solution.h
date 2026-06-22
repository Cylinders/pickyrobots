#pragma once
#include <optional>
#include <string>

#include "pos.h"

namespace mapf {
    using AgentSolution = std::vector<Pos>;

    struct Solution {
        std::string map;
        std::string scenario;
        std::string algo;
        double time; // if time == -1, the algorithm did not compute the solution in the time limit.
        bool completed;
        std::vector<std::optional<AgentSolution>> agent_solutions;
    };
}
