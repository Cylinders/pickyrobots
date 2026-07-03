/*
This file is part of BCP-MAPF.

BCP-MAPF is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

BCP-MAPF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BCP-MAPF.  If not, see <https://www.gnu.org/licenses/>.

Author: Edward Lam <ed@ed-lam.com>
*/

#include "problem/output.h"
#include "problem/includes.h"
#include "problem/problem.h"
#include "problem/variable_data.h"
#include <string> // Required for std::string

#include <fmt/core.h>
#include <string>
#include <vector>
#include "mapf_common/solution.h"


mapf::Solution write_best_solution(
    SCIP* scip,
    mapf::Solution& mapf_sol
)
{
    debug_assert(scip);

    auto probdata = SCIPgetProbData(scip);
    const auto N = SCIPprobdataGetN(probdata);

    const auto& map = SCIPprobdataGetMap(probdata);

    mapf_sol.agent_solutions.assign(N, std::nullopt);
    mapf_sol.completed = false;
    mapf_sol.time = SCIPgetSolvingTime(scip) * 1000.0;

    const auto& dummy_vars = SCIPprobdataGetDummyVars(probdata);
    const auto& agent_vars = SCIPprobdataGetAgentVars(probdata);

    // Get best solution.
    auto sol = SCIPgetBestSol(scip);
    SCIP_Real obj = 0;

    if (!sol)
    {
        mapf_sol.completed = false;
        return mapf_sol;
    }

    obj = SCIPgetSolOrigObj(scip, sol);
    if (obj >= ARTIFICIAL_VAR_COST)
    {
        mapf_sol.completed = false;
        return mapf_sol;
    }

    for (Agent a = 0; a < N; ++a)
    {
        auto var = dummy_vars[a];
        debug_assert(var);
        debug_assert(!SCIPvarGetData(var));

        const auto var_val = SCIPgetSolVal(scip, sol, var);
    }

    // Extract the actual paths
    for (Agent a = 0; a < N; ++a)
    {
        bool found = false;

        for (const auto& [var, _] : agent_vars[a])
        {
            debug_assert(var);

            const auto var_val = SCIPgetSolVal(scip, sol, var);

            if (SCIPisPositive(scip, var_val))
            {
                auto vardata = SCIPvarGetData(var);
                const auto path_length = SCIPvardataGetPathLength(vardata);
                const auto path = SCIPvardataGetPath(vardata);

                mapf::AgentSolution current_agent_path;
                current_agent_path.reserve(path_length);

                for (int t = 0; t < path_length; ++t)
                {
                    const auto e = path[t];
                    auto [x, y] = map.get_xy(e.n);

#ifdef REMOVE_PADDING
                    --x;
                    --y;
#endif

                    current_agent_path.push_back({x, y});
                }

                mapf_sol.agent_solutions[a] = std::move(current_agent_path);
                found = true;

                break;
            }
        }

        if (!found)
        {
            mapf_sol.completed = false;
            return mapf_sol;
        }
    }

    mapf_sol.completed = true;
    return mapf_sol;
}
