#pragma once

#include <span>
#include <unordered_map>

#include "agent.h"
#include "grid.h"
#include "portfolio.h"

struct ValuationInput {
    const Grid& grid;
    const std::vector<Agent> agents;
    const std::unordered_map<Agent, std::vector<Pos>>& paths;
};

struct SolverWeight {
    SolverKind solver;
    double weight;
};

template <typename Portfolio> struct Valuation {
    virtual ~Valuation() = default;

    [[nodiscard]]
    virtual std::array<SolverWeight, Portfolio::solvers.size()> evaluate(const ValuationInput& input) = 0;
};