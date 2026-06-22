#pragma once

#include <filesystem>
<<<<<<< HEAD
#include <fstream>
=======
>>>>>>> 788279b (bcp and cbs and cbsh all COMPILE (not tested) to output a solution TYPE)

#include "grid.h"

namespace mapf::reader {
    Grid read_map(std::istream &in);
<<<<<<< HEAD

    inline Grid read_map(const std::filesystem::path &path) {
        std::ifstream file{path};

        if (!file) {
            throw std::runtime_error("failed to open scenario file");
        }

        return read_map(file);
    }
=======
>>>>>>> 788279b (bcp and cbs and cbsh all COMPILE (not tested) to output a solution TYPE)
}
