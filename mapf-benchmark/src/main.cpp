#include <iostream>
#include <queue>
#include <random>
#include <onnxruntime_cxx_api.h>
#include <solvers.h>
#include <thread>
#include <mutex>

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

    // Create a mutex to protect our output streams
    std::mutex io_mutex;

    // Ensure the solutions directory exists
    std::filesystem::path solutions_dir = "../solutions";
    std::filesystem::create_directories(solutions_dir);

    for (const auto &scenario_path: problem.scenarios) {
        std::cout << "map: " << problem.map << ", scenario: " << scenario_path << std::endl;
        out << "scenario: " << scenario_path << std::endl;

        const std::vector<mapf::Agent> scenario = mapf::reader::read_scenario(scenario_path);
        const auto paths = find_minimal_paths(map, scenario);

        // --- MAPFASTER VALUATOR TIMING ---
        auto val_start = std::chrono::steady_clock::now();
        auto valuation = valuator.evaluate({map, scenario, paths});
        auto val_end = std::chrono::steady_clock::now();
        auto val_elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(val_end - val_start).count();

        out << "MAPFASTER: " << val_elapsed_ms << " ms" << std::endl;

        for (auto [solver, weight]: valuation) {
            out << to_string(solver) << ":" << weight << "" << std::endl;
        }

        // --- MULTITHREADING SECTION ---

        // Start timer for ALL threads
        auto threads_start = std::chrono::steady_clock::now();

        // 1. CBS Thread
        std::thread t_cbs([&]() {
            {
                std::lock_guard<std::mutex> lock(io_mutex);
                std::cout << "starting cbs\n";
            }

            auto start = std::chrono::steady_clock::now();
            mapf::Solution solved = cbs_solve(problem.map, scenario_path, scenario.size());
            auto end = std::chrono::steady_clock::now();
            solved.map = problem.map;
            solved.scenario = scenario_path;
            solved.algo = "CBS";
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            // solved.time = elapsed_ms;
            // Write solution to file unconditionally
            std::string sol_filename = scenario_path.stem().string() + "_cbs.sol";
            std::ofstream sol_out(solutions_dir / sol_filename);
            mapf::writer::write_solution(sol_out, solved);

            {
                std::lock_guard<std::mutex> lock(io_mutex);
                if (!solved.completed) { out << "CBS failed." << std::endl; }
                else { out << "CBS took " << elapsed_ms << " ms" << std::endl; }
            }
        });

        // 2. CBSH Thread
        std::thread t_cbsh([&]() {
            {
                std::lock_guard<std::mutex> lock(io_mutex);
                std::cout << "starting cbsh\n";
            }

            auto start = std::chrono::steady_clock::now();
            mapf::Solution solved = cbsh_solve(problem.map, scenario_path, scenario.size());
            auto end = std::chrono::steady_clock::now();
            solved.map = problem.map;
            solved.scenario = scenario_path;
            solved.algo = "CBSH";
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            // solved.time = elapsed_ms;

            // Write solution to file unconditionally
            std::string sol_filename = scenario_path.stem().string() + "_cbsh.sol";
            std::ofstream sol_out(solutions_dir / sol_filename);
            mapf::writer::write_solution(sol_out, solved);

            {
                std::lock_guard<std::mutex> lock(io_mutex);
                if (!solved.completed) { out << "CBSH failed." << std::endl; }
                else { out << "CBSH took " << elapsed_ms << " ms" << std::endl; }
            }
        });

        // 3. BCP Thread
        std::thread t_bcp([&]() {
            {
                std::lock_guard<std::mutex> lock(io_mutex);
                std::cout << "starting bcp\n";
            }

            scip_tmp_copy(problem.map, scenario_path);

            auto start = std::chrono::steady_clock::now();
            mapf::Solution solved = bcp_solve(60, std::filesystem::absolute(tmp / scenario_path.filename()));
            auto end = std::chrono::steady_clock::now();
            solved.map = problem.map;
            solved.scenario = scenario_path;
            solved.algo = "BCP";
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            // solved.time = elapsed_ms;
            // Write solution to file unconditionally
            std::string sol_filename = scenario_path.stem().string() + "_bcp.sol";
            std::ofstream sol_out(solutions_dir / sol_filename);
            mapf::writer::write_solution(sol_out, solved);

            {
                std::lock_guard<std::mutex> lock(io_mutex);
                if (!solved.completed) { out << "BCP failed." << std::endl; }
                else { out << "BCP took " << elapsed_ms << " ms" << std::endl; }
            }
        });

        // --- SYNCHRONIZATION ---

        // Wait for all three solvers to finish before looping to the next scenario
        if (t_cbs.joinable()) t_cbs.join();
        if (t_cbsh.joinable()) t_cbsh.join();
        if (t_bcp.joinable()) t_bcp.join();

        // End timer for ALL threads and output to file
        auto threads_end = std::chrono::steady_clock::now();
        auto threads_elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(threads_end - threads_start).count();

        out << "ALL took " << threads_elapsed_ms << " ms" << std::endl;
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
