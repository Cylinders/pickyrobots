#include <iostream>
#include <queue>
#include <random>
#include <onnxruntime_cxx_api.h>

#include "agent.h"
#include "grid.h"
#include "manifest.h"
#include "mapfaster_encoder.h"
#include "mapfaster_valuation.h"

std::unordered_map<Agent, std::vector<Pos> > find_minimal_paths(const Grid &grid, const std::vector<Agent> &agents);

std::vector<Agent> sample_n(const std::vector<Agent> &agents, std::size_t n);

int main() {
    std::cout << "mapf benchmarker" << std::endl;

    MapfasterValuation valuator{"../data/model/None_dummy-93iq1npl.onnx"};

    const auto problems = parse_manifest("../data/problems/manifest.jsonl");

    for (const auto& problem : problems) {
        std::cout << problem.id << '\n';
        std::cout << "  map: " << problem.map << '\n';
        const Grid map = parse_map_from(problem.map);

        for (const auto& scenario_path : problem.scenarios) {
            std::cout << "  scenario: " << scenario_path << '\n';
            const std::vector<Agent> scenario = sample_n(parse_scenario_from(scenario_path), 50);
            const auto paths = find_minimal_paths(map, scenario);

            auto valuation = valuator.evaluate({ map, scenario, paths});

            for (auto [solver, weight] : valuation) {
                std::cout << to_string(solver) << " " << weight << "\n";
            }
        }
    }
}

std::vector<Agent> sample_n(const std::vector<Agent> &agents, std::size_t n) {
    std::vector<Agent> result;
    result.reserve(n);

    std::ranges::sample(agents, std::back_inserter(result), n, std::mt19937{std::random_device{}()});

    return result;
}

std::unordered_map<Agent, std::vector<Pos> > find_minimal_paths(const Grid &grid, const std::vector<Agent> &agents) {
    std::unordered_map<Agent, std::vector<Pos> > paths;

    const int h = grid.height;
    const int w = grid.width;

    auto in_bounds = [&](const Pos &p) {
        return 0 <= p.row && p.row < h &&
               0 <= p.col && p.col < w;
    };

    auto index = [&](const Pos &p) {
        return static_cast<std::size_t>(p.row * w + p.col);
    };

    auto from_index = [&](const int i) {
        return Pos{
            .row = i / w,
            .col = i % w,
        };
    };

    constexpr Pos dirs[] = {
        Pos{.row = -1, .col = 0},
        Pos{.row = 1, .col = 0},
        Pos{.row = 0, .col = -1},
        Pos{.row = 0, .col = 1},
    };

    for (const Agent &agent: agents) {
        const Pos start = agent.start;
        const Pos goal = agent.goal;

        if (grid.is_blocked(start.col, start.row) || grid.is_blocked(goal.col, goal.row)) {
            throw std::runtime_error("agent start/goal is blocked");
        }

        std::vector<int> parent(h * w, -1);
        std::queue<Pos> q;

        const int start_i = index(start);
        const int goal_i = index(goal);

        parent[start_i] = start_i;
        q.push(start);

        while (!q.empty()) {
            const Pos cur = q.front();
            q.pop();

            const int cur_i = index(cur);
            if (cur_i == goal_i) {
                break;
            }

            for (const auto &[row, col]: dirs) {
                const Pos next{
                    .row = cur.row + row,
                    .col = cur.col + col,
                };

                if (!in_bounds(next) || grid.is_blocked(next.col, next.row)) {
                    continue;
                }

                const int next_i = index(next);
                if (parent[next_i] != -1) {
                    continue;
                }

                parent[next_i] = cur_i;
                q.push(next);
            }
        }

        if (parent[goal_i] == -1) {
            paths.emplace(agent, std::vector<Pos>{});
            continue;
        }

        std::vector<Pos> path;
        for (int at = goal_i; at != start_i; at = parent[at]) {
            path.push_back(from_index(at));
        }
        path.push_back(start);

        std::ranges::reverse(path);

        paths.emplace(agent, std::move(path));
    }

    return paths;
}
