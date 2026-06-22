#pragma once

#include <filesystem>

#include "solution.h"

namespace mapf::writer {
    void write_solution(std::ostream &out, const Solution &solution);
<<<<<<< HEAD

    inline void write_solution(const std::filesystem::path &path, const Solution &solution) {
        std::ofstream file{path};

        if (!file) {
            throw std::runtime_error("failed to open solution file");
        }

        return write_solution(file, solution);
    }
=======
>>>>>>> 788279b (bcp and cbs and cbsh all COMPILE (not tested) to output a solution TYPE)
}
