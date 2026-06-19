#pragma once

#include <vector>

namespace mapf {
    struct Grid {
        int width;
        int height;
        std::vector<bool> blocked;

        [[nodiscard]] std::size_t index(const int row, const int col) const {
            return static_cast<std::size_t>(row * width + col);
        }

        [[nodiscard]] bool is_blocked(const int row, const int col) const {
            return blocked[index(row, col)];
        }
    };
}
