#pragma once

<<<<<<< HEAD
#include <filesystem>
#include <fstream>

#include "agent.h"

namespace mapf::reader {
    Agents read_scenario(std::istream &in);

    inline Agents read_scenario(const std::filesystem::path &path) {
        std::ifstream file{path};

        if (!file) {
            throw std::runtime_error("failed to open scenario file");
        }

        return read_scenario(file);
    }
=======
#include <fstream>
#include <mapf-common/agent.h>

namespace mapf::reader {
    Agents read_scenario(std::istream &in);
>>>>>>> 788279b (bcp and cbs and cbsh all COMPILE (not tested) to output a solution TYPE)
}
