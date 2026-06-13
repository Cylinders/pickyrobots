#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "pos.h"

struct Agent {
    Pos start;
    Pos goal;

    auto operator<=>(const Agent&) const = default;
};

template <>
struct std::hash<Agent> {
    std::size_t operator()(const Agent& a) const noexcept {
        auto hs = std::hash<Pos>{}(a.start);
        auto hg = std::hash<Pos>{}(a.goal);
        return hs ^ hg << 1;
    }
};

inline std::ostream& operator<<(std::ostream& os, const Pos& pos) {
    return os << "(" << pos.row << ", " << pos.col << ")";
}

std::vector<Agent> parse_scenario(std::istream& in);

inline std::vector<Agent> parse_scenario(const std::string_view text) {
    std::istringstream in{std::string{text}};
    return parse_scenario(in);
}

inline std::vector<Agent> parse_scenario_from(const std::filesystem::path& path) {
    std::ifstream file{path};

    if (!file) {
        throw std::runtime_error("failed to open scenario file");
    }

    return parse_scenario(file);
}

inline void dump_scenario(std::ostream& os, const std::vector<Agent>& scenario) {
    for (auto [start, goal] : scenario) {
        os << start << " " << goal << "\n";
    }
}