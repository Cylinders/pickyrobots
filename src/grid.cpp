#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "grid.h"

inline bool is_passable(const char c) {
    return c == '.' || c == 'G' || c == 'S' || c == 'W';
}

inline bool is_blocked(const char c) {
    return c == '@' || c == 'O' || c == 'T';
}

Grid parse_map(std::istream &in) {
    std::string key;
    std::string value;

    in >> key >> value;
    if (key != "type" || value != "octile") {
        throw std::runtime_error("expected header: type octile");
    }

    int height = 0;
    in >> key >> height;
    if (key != "height" || height <= 0) {
        throw std::runtime_error("invalid height header");
    }

    int width = 0;
    in >> key >> width;
    if (key != "width" || width <= 0) {
        throw std::runtime_error("invalid width header");
    }

    in >> key;
    if (key != "map") {
        throw std::runtime_error("expected header: map");
    }

    Grid grid{
        .width = width,
        .height = height,
        .blocked = std::vector<std::uint8_t>(
            width * height,
            0
        ),
    };

    std::string row;

    for (int y = 0; y < height; ++y) {
        if (!(in >> row)) {
            throw std::runtime_error("expected more map rows");
        }

        if (static_cast<int>(row.size()) != width) {
            throw std::runtime_error("map row has invalid width");
        }

        for (int x = 0; x < width; ++x) {
            const char c = row[x];

            if (is_passable(c)) {
                continue;
            }

            if (is_blocked(c)) {
                grid.blocked[grid.index(x, y)] = 1;
                continue;
            }

            throw std::runtime_error("invalid map character");
        }
    }

    if (in >> row) {
        throw std::runtime_error("too many map rows");
    }

    return grid;
}

void dump_grid(std::ostream &os, const Grid &grid) {
    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            os << (grid.blocked[y * grid.width + x] ? '@' : '.');
        }
        os << '\n';
    }
}
