#include "agent.h"

std::vector<Agent> parse_scenario(std::istream &in) {
    std::string line;

    if (!std::getline(in, line)) {
        throw std::runtime_error("empty scenario");
    }

    std::istringstream header(line);

    if (std::string version, word; !(header >> word >> version) || word != "version") {
        throw std::runtime_error("scenario must start with 'version x.x'");
    }

    std::vector<Agent> agents;
    int line_no = 1;

    while (std::getline(in, line)) {
        ++line_no;

        if (line.empty()) {
            continue;
        }

        std::istringstream ss(line);

        int bucket;
        std::string map_name;
        int map_width;
        int map_height;
        int start_x;
        int start_y;
        int goal_x;
        int goal_y;
        double optimal_length;

        if (!(ss >> bucket
              >> map_name
              >> map_width
              >> map_height
              >> start_x
              >> start_y
              >> goal_x
              >> goal_y
              >> optimal_length)) {
            throw std::runtime_error(
                "line " + std::to_string(line_no) + ": expected 9 fields"
            );
        }

        std::string extra;
        if (ss >> extra) {
            throw std::runtime_error(
                "line " + std::to_string(line_no) + ": expected 9 fields"
            );
        }

        agents.push_back(Agent{
            .start = Pos{.row = start_y, .col = start_x},
            .goal = Pos{.row = goal_y, .col = goal_x},
        });
    }

    return agents;
}
