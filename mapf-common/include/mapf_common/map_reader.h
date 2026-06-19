#pragma once

#include <filesystem>

#include "grid.h"

namespace mapf::reader {
    Grid read_map(std::istream &in);
}
