#pragma once

#include <fstream>
#include <mapf_common/agent.h>

namespace mapf::reader {
    Agents read_scenario(std::istream &in);
}