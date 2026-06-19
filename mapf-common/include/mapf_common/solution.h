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
        std::vector<std::optional<AgentSolution>> agent_solutions;
    };

}
