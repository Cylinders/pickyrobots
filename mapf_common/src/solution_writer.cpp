#include <fstream>
#include <mapf_common/solution_writer.h>
#include <string>

namespace {
    void write_agent_solution(std::ostream &out, const std::optional<mapf::AgentSolution> &solution) {
        if (!solution) {
            out << "x";
            return;
        }

        for (auto [row, col] : *solution) {
            out << col << "," << row << " ";
        }

        out << std::endl;
    }
}

void mapf::writer::write_solution(std::ostream &out, const Solution &solution) {
    out << solution.map << std::endl;
    out << solution.scenario << std::endl;
    out << solution.algo << std::endl;
    if (solution.completed) {
        out << solution.time << std::endl;
        for (const auto &agent_solution : solution.agent_solutions) {
                        write_agent_solution(out, agent_solution);
            }
    }
    else {
         out << "-1"<< std::endl;
    }


}
