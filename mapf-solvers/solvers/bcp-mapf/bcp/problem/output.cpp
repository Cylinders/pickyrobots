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

// Make sure your mapf:: headers are included
// #include "solution.h"
// #include "pos.h"

mapf::Solution write_best_solution(
    SCIP* scip,
    mapf::Solution& mapf_sol
)
{
    // Check.
    debug_assert(scip);

    // Get problem data.
    auto probdata = SCIPgetProbData(scip);
    const auto N = SCIPprobdataGetN(probdata);

    // Initialize base MAPF Solution data.
    mapf_sol.agent_solutions.assign(N, std::nullopt);
    mapf_sol.completed = false;
    mapf_sol.time = SCIPgetSolvingTime(scip);

    // Get variables.
    const auto& dummy_vars = SCIPprobdataGetDummyVars(probdata);
    const auto& agent_vars = SCIPprobdataGetAgentVars(probdata);

    // Get best solution.
    auto sol = SCIPgetBestSol(scip);
    SCIP_Real obj = 0;

    if (!sol)
    {
        mapf_sol.completed = false;
    }

    // Exit if no solution within objective limit is found.
    obj = SCIPgetSolOrigObj(scip, sol);
    if (obj >= ARTIFICIAL_VAR_COST)
    {
        mapf_sol.completed = false;
    }

    // Note: This helper function might still print to standard output internally.
    print_used_paths(scip, sol);

    // Check if dummy variables are used.
    for (Agent a = 0; a < N; ++a)
    {
        auto var = dummy_vars[a];
        debug_assert(var);
        debug_assert(!SCIPvarGetData(var));

        const auto var_val = SCIPgetSolVal(scip, sol, var);
    }

    // If we pass the dummy variable check, we have a complete and valid solution.
    mapf_sol.completed = true;

    // Initialize the output string with the objective value.
    std::string output_str = fmt::format("{:.0f}\n\n", SCIPround(scip, obj));

    // Append paths to the string and populate the mapf::Solution object.
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

                // 1. Get the path as a string (e.g., "(28,4) (28,5) (28,6)")
                std::string path_str = format_path(probdata, path_length, path);

                // 2. Append the specific agent's path to our output string.
                output_str += fmt::format("Agent {}, cost {:.0f}, path {}\n",
                                          a,
                                          SCIPround(scip, SCIPvarGetObj(var)),
                                          path_str);

                // 3. Parse the string to populate mapf_sol
                mapf::AgentSolution current_agent_path;
                current_agent_path.reserve(path_length);

                size_t pos = 0;
                while ((pos = path_str.find('(', pos)) != std::string::npos)
                {
                    size_t comma = path_str.find(',', pos);
                    size_t end_paren = path_str.find(')', comma);

                    if (comma != std::string::npos && end_paren != std::string::npos)
                    {
                        // Extract the row and col substrings and convert to integers
                        int r = std::stoi(path_str.substr(pos + 1, comma - pos - 1));
                        int c = std::stoi(path_str.substr(comma + 1, end_paren - comma - 1));

                        current_agent_path.push_back({r, c});
                        pos = end_paren + 1; // Move past the current coordinate
                    }
                    else
                    {
                        break; // Malformed string, stop parsing
                    }
                }

                mapf_sol.agent_solutions[a] = std::move(current_agent_path);

                // Move to next agent.
                release_assert(!found, "Agent {} is using more than one path", a);
                found = true;
                break;
            }
        }
        release_assert(found, "Agent {} has no path in the solution", a);
    }

    return mapf_sol;
}
