#pragma once

#include <filesystem>

#include "solution.h"

namespace mapf::writer {
    void write_solution(std::ostream &out, const Solution &solution);
}
