#pragma once

struct Pos {
    int row{};
    int col{};

    auto operator<=>(const Pos&) const = default;
};

template <>
struct std::hash<Pos> {
    std::size_t operator()(const Pos& p) const noexcept {
        auto hrow = std::hash<int>{}(p.row);
        auto hcol = std::hash<int>{}(p.col);
        return hrow ^ (hcol << 1);
    }
};
