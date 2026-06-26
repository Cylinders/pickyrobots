#pragma once
#include <array>

enum class SolverKind {
    CBS,
    CBSH,
    BCP,
    MDDSAT
};

constexpr std::string_view to_string(SolverKind kind) {
    switch (kind) {
        case SolverKind::CBS: return "CBS";
        case SolverKind::CBSH: return "CBSH";
        case SolverKind::BCP: return "BCP";
        case SolverKind::MDDSAT: return "MDDSAT";
    }

    return "<unknown>";
}

struct MapfasterPortfolio {
    static constexpr std::array solvers = {
        SolverKind::CBS,
        SolverKind::CBSH,
        SolverKind::BCP,
        SolverKind::MDDSAT
    };
};
