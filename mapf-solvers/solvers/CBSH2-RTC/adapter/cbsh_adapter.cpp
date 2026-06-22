#include "cbsh_adapter.h"
#include <iostream>
#include <string>
#include "CBS.h"
#include "CBSHeuristic.h"
#include "mapf_common/solution.h"

namespace {
    mapf::Solution cbsh_solve_with_params(heuristics_type h,        // Options: "Zero", "CG", "DG", "WDG"
                                rectangle_strategy r,    // Options: "None", "R", "RM", "GR", "Disjoint"
                                corridor_strategy c,    // Options: "None", "C", "PC", "STC", "GC", "Disjoint"
                                const std::string &mapFilePath, const std::string &scenFilePath, int n, const std::string &algo) {
        // Other solver configurations
        int agentNum = 0; // 0 reads all agents from the file
        double cutoffTime = 60.0; // seconds
        int screenOption = 0; // 0: none, 1: results, 2: all
        int seed = 0;
        int restartRuns = 1;
        bool usingSipp = false;

        // --- Load the Instance ---
        // Assuming constructor matches: (map_file, agents_file, num_agents, agent_indices, rows, cols, obs, warehouse_width)
        Instance instance(mapFilePath, scenFilePath, n, "", 0, 0, 0, 0);
        srand(seed);

        // --- Initialize the Solver ---
        CBS cbs(instance, usingSipp, screenOption);
        cbs.setPrioritizeConflicts(true);
        cbs.setDisjointSplitting(false);
        cbs.setBypass(true);
        cbs.setRectangleReasoning(r);
        cbs.setCorridorReasoning(c);
        cbs.setHeuristicType(h);
        cbs.setTargetReasoning(true);
        cbs.setMutexReasoning(false);
        cbs.setSavingStats(false);
        // cbs.setNodeLimit(MAX_NODES); // Uncomment if MAX_NODES is defined in your headers

        // --- Run CBS ---
        double runtime = 0;
        int min_f_val = 0;
        bool solved;
        for (int i = 0; i < restartRuns; i++) {
            cbs.clear();
            solved = cbs.solve(cutoffTime, min_f_val);
            runtime += cbs.runtime;
            if (cbs.solution_found) break;
            min_f_val = (int) cbs.min_f_val;
            cbs.randomRoot = true;
        }
        cbs.runtime = runtime;
        cbs.clearSearchEngines();

        return cbs.returnPaths(mapFilePath, scenFilePath, algo, solved, runtime);
    }

}

namespace mapf_solvers::cbsh {
    mapf::Solution cbs_solve(const std::string &mapFilePath, const std::string &scenFilePath, const int n) {
        return cbsh_solve_with_params(ZERO, NR, NC, mapFilePath, scenFilePath, n, "CBS");
    }

    mapf::Solution cbsh_solve(const std::string &mapFilePath, const std::string &scenFilePath, const int n) {
        return cbsh_solve_with_params(WDG, GR, GC, mapFilePath, scenFilePath, n, "CBSH");
    }

    /*
    mapf::Solution cbsh_solveReturn(const std::string &mapFilePath, const std::string &scenFilePath, int n){

    }
    */
}
