#include <iostream>
#include <queue>
#include <random>
#include <onnxruntime_cxx_api.h>

#include "agent.h"
#include "grid.h"
#include "mapfaster_encoder.h"

std::unordered_map<Agent, std::vector<Pos> > find_minimal_paths(const Grid &grid, const std::vector<Agent> &agents);

std::vector<Agent> sample_n(const std::vector<Agent> &agents, std::size_t n);

int main() {
    std::cout << "mapf benchmarker" << std::endl;

    const Grid map = parse_map_from("../data/8room_000.map");
    dump_grid(std::cout, map);

    const std::vector<Agent> scenario = sample_n(parse_scenario_from("../data/8room_000.map.scen"), 50);
    dump_scenario(std::cout, scenario);

    const auto paths = find_minimal_paths(map, scenario);

    auto input_buffer = mapfaster_encode(map, scenario, paths);

    std::array<int64_t, 4> input_shape = {
        1, 3, IMAGE_SIZE, IMAGE_SIZE
    };

    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
        OrtArenaAllocator,
        OrtMemTypeDefault
    );

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        input_buffer.data(),
        input_buffer.size(),
        input_shape.data(),
        input_shape.size()
    );

    Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "mapfaster"};
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);

    Ort::Session session{
        env,
        "../data/model/None_dummy-93iq1npl.onnx",
        session_options
    };

    Ort::AllocatorWithDefaultOptions allocator;

    auto input_name = session.GetInputNameAllocated(0, allocator);
    auto output_name = session.GetOutputNameAllocated(0, allocator);

    const char *input_names[] = {input_name.get()};
    const char *output_names[] = {output_name.get()};

    auto outputs = session.Run(
        Ort::RunOptions{nullptr},
        input_names,
        &input_tensor,
        1,
        output_names,
        1
    );

    float *y = outputs[0].GetTensorMutableData<float>();

    auto output_info = outputs[0].GetTensorTypeAndShapeInfo();
    std::vector<int64_t> output_shape = output_info.GetShape();

    size_t output_count = output_info.GetElementCount();

    for (size_t i = 0; i < output_count; ++i) {
        std::cout << y[i] << '\n';
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
