#pragma once

#include <mapf_common/grid.h>
#include <mapf_common/agent.h>

#include "portfolio.h"

struct ValuationInput {
    const mapf::Grid& grid;
    const std::vector<mapf::Agent> agents;
    const std::unordered_map<mapf::Agent, std::vector<mapf::Pos>>& paths;
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