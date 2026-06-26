#pragma once

#include <unordered_map>

#include "mapf_common/agent.h"
#include "mapf_common/grid.h"

constexpr auto IMAGE_SIZE = 320;

std::vector<float> mapfaster_encode(const mapf::Grid &grid, const std::vector<mapf::Agent>& agents, const std::unordered_map<mapf::Agent, std::vector<mapf::Pos>>& paths);

