#pragma once

#include <filesystem>
#include <fstream>
#include <vector>

struct Grid {
    int width;
    int height;
    std::vector<std::uint8_t> blocked;

    [[nodiscard]] std::size_t index(const int x, const int y) const {
        return static_cast<std::size_t>(y * width + x);
    }

    [[nodiscard]] bool is_blocked(const int x, const int y) const {
        return blocked[index(x, y)] != 0;
    }};

Grid parse_map(std::istream& in);

inline Grid parse_map(const std::string_view text) {
    std::istringstream in{std::string{text}};
    return parse_map(in);
}

inline Grid parse_map_from(const std::filesystem::path& path) {
    std::ifstream file{path};

    if (!file) {
        throw std::runtime_error("failed to open map file");
    }

    return parse_map(file);
}

void dump_grid(std::ostream& os, const Grid& grid);