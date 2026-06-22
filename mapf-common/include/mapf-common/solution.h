#pragma once
#include <optional>
#include <string>

#include "pos.h"

namespace mapf {
    using AgentSolution = std::vector<Pos>;

    struct Solution {
        double time;
        bool completed;
        std::string map;
        std::string scenario;
        std::string algo;
        std::vector<std::optional<AgentSolution>> agent_solutions;
    };
}
