#pragma once

#include <cstddef>
#include <functional>

namespace mapf {
    struct Pos {
        int row{};
        int col{};

        auto operator<=>(const Pos&) const = default;
    };

}

template <>
struct std::hash<mapf::Pos> {
    std::size_t operator()(const mapf::Pos& p) const noexcept {
        auto hrow = std::hash<int>{}(p.row);
        auto hcol = std::hash<int>{}(p.col);
        return hrow ^ (hcol << 1);
    }
};