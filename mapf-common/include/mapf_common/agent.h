#pragma once

#include "pos.h"

namespace mapf {
    struct Agent {
        Pos start;
        Pos goal;

        auto operator<=>(const Agent&) const = default;
    };

    using Agents = std::vector<Agent>;
}
