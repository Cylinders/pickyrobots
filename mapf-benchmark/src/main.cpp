#include <iostream>
#include <queue>
#include <random>
#include <onnxruntime_cxx_api.h>
#include <solvers.h>

#include "manifest.h"
#include "mapfaster_encoder.h"
#include "mapfaster_valuation.h"
#include "mapf_common/map_reader.h"
#include "mapf_common/scenario_reader.h"
#include "mapf_common/solution.h"
#include "mapf_common/solution_writer.h"

std::unordered_map<mapf::Agent, std::vector<mapf::Pos> > find_minimal_paths(
    const mapf::Grid &grid, const std::vector<mapf::Agent> &agents);

std::vector<mapf::Agent> sample_n(const std::vector<mapf::Agent> &agents, std::size_t n);

const std::filesystem::path output = "../output";

const std::filesystem::path tmp = "../tmp";

void scip_tmp_copy(const std::filesystem::path &f1, const std::filesystem::path &f2);

void scip_tmp_clear();

void benchmark_problem(MapfasterValuation &valuator, const ManifestProblem &problem) {
    std::ofstream out(output / (problem.id + ".txt"));

    out << problem.id << '\n';
    out << "map: " << problem.map << std::endl;
    const mapf::Grid map = mapf::reader::read_map(problem.map);

    for (const auto &scenario_path: problem.scenarios) {
        std::cout << "map: " << problem.map << ", scenario: " << scenario_path << std::endl;
        out << "scenario: " << scenario_path << std::endl;

        // const std::vector<Agent> scenario = sample_n(parse_scenario_from(scenario_path), 2);
        const std::vector<mapf::Agent> scenario = mapf::reader::read_scenario(scenario_path);
        const auto paths = find_minimal_paths(map, scenario);

        auto valuation = valuator.evaluate({map, scenario, paths});

        for (auto [solver, weight]: valuation) {
            out << to_string(solver) << ":" << weight << "" << std::endl;
        }

        // cbs  1
        std::cout << "starting cbs\n";
        {
            auto start = std::chrono::steady_clock::now();
            mapf::Solution solved = cbs_solve(problem.map, scenario_path, scenario.size());
            auto end = std::chrono::steady_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            if (!solved.completed) { out << "CBS failed." << std::endl; } else {
                out << "CBS took " << elapsed_ms << " ms" << std::endl;
            }
        }

        // cbsh 2
        std::cout << "starting cbsh\n";
        {
            auto start = std::chrono::steady_clock::now();
            mapf::Solution solved = cbsh_solve(problem.map, scenario_path, scenario.size());
            auto end = std::chrono::steady_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            if (!solved.completed) { out << "CBSH failed." << std::endl; } else {
                out << "CBSH took " << elapsed_ms << " ms" << std::endl;
            }
        }

        // bcp  3
        std::cout << "starting bcp\n";

        {
            scip_tmp_copy(problem.map, scenario_path);

            auto start = std::chrono::steady_clock::now();
            mapf::Solution output = bcp_solve(60, std::filesystem::absolute(tmp / scenario_path.filename()));
            auto end = std::chrono::steady_clock::now();

            scip_tmp_clear();

            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            if (!output.completed) {
                out << "BCP failed." << std::endl;
            } else {
                out << "BCP took " << elapsed_ms << " ms" << std::endl;
            }
        }
    }

    out.close();
}

int main() {
    std::cout << "mapf benchmarker" << std::endl;

    MapfasterValuation valuator{"../data/model/None_dummy-93iq1npl.onnx"};

    const auto problems = parse_manifest("../data/problems/manifest.jsonl");
    std::filesystem::create_directories(output);

    // warmup
    for (int i = 0; i < 1; ++i) {
        benchmark_problem(valuator, problems[0]);
    }

    for (const auto &problem: problems) {
        benchmark_problem(valuator, problem);
    }
}

void scip_tmp_copy(const std::filesystem::path &f1, const std::filesystem::path &f2) {
    std::filesystem::create_directories(tmp);

    std::filesystem::copy_file(f1, tmp / f1.filename(), std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy_file(f2, tmp / f2.filename(), std::filesystem::copy_options::overwrite_existing);
}

void scip_tmp_clear() {
    for (const auto &entry: std::filesystem::directory_iterator(tmp)) {
        std::filesystem::remove(entry.path());
    }
}

std::vector<mapf::Agent> sample_n(const std::vector<mapf::Agent> &agents, std::size_t n) {
    std::vector<mapf::Agent> result;
    result.reserve(n);

    std::ranges::sample(agents, std::back_inserter(result), n, std::mt19937{std::random_device{}()});

    return result;
}

std::unordered_map<mapf::Agent, std::vector<mapf::Pos> > find_minimal_paths(
    const mapf::Grid &grid, const std::vector<mapf::Agent> &agents) {
    std::unordered_map<mapf::Agent, std::vector<mapf::Pos> > paths;

    const int h = grid.height;
    const int w = grid.width;

    auto in_bounds = [&](const mapf::Pos &p) {
        return 0 <= p.row && p.row < h &&
               0 <= p.col && p.col < w;
    };

    auto index = [&](const mapf::Pos &p) {
        return static_cast<std::size_t>(p.row * w + p.col);
    };

    auto from_index = [&](const int i) {
        return mapf::Pos{
            .row = i / w,
            .col = i % w,
        };
    };

    constexpr mapf::Pos dirs[] = {
        mapf::Pos{.row = -1, .col = 0},
        mapf::Pos{.row = 1, .col = 0},
        mapf::Pos{.row = 0, .col = -1},
        mapf::Pos{.row = 0, .col = 1},
    };

    for (const mapf::Agent &agent: agents) {
        const mapf::Pos start = agent.start;
        const mapf::Pos goal = agent.goal;

        if (grid.is_blocked(start.row, start.col) || grid.is_blocked(goal.row, goal.col)) {
            throw std::runtime_error("agent start/goal is blocked");
        }

        std::vector<int> parent(h * w, -1);
        std::queue<mapf::Pos> q;

        const int start_i = index(start);
        const int goal_i = index(goal);

        parent[start_i] = start_i;
        q.push(start);

        while (!q.empty()) {
            const mapf::Pos cur = q.front();
            q.pop();

            const int cur_i = index(cur);
            if (cur_i == goal_i) {
                break;
            }

            for (const auto &[row, col]: dirs) {
                const mapf::Pos next{
                    .row = cur.row + row,
                    .col = cur.col + col,
                };

                if (!in_bounds(next) || grid.is_blocked(next.row, next.col)) {
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
            paths.emplace(agent, std::vector<mapf::Pos>{});
            continue;
        }

        std::vector<mapf::Pos> path;
        for (int at = goal_i; at != start_i; at = parent[at]) {
            path.push_back(from_index(at));
        }
        path.push_back(start);

        std::ranges::reverse(path);

        paths.emplace(agent, std::move(path));
    }

    return paths;
}
