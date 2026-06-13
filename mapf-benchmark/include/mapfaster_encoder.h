#pragma once

#include <unordered_map>
#include <opencv2/imgproc.hpp>

#include "agent.h"
#include "grid.h"

constexpr auto IMAGE_SIZE = 320;

std::vector<float> mapfaster_encode(const Grid &grid, const std::vector<Agent>& agents, const std::unordered_map<Agent, std::vector<Pos>>& paths);

