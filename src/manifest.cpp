#include "manifest.h"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<ManifestProblem> parse_manifest(
    const std::filesystem::path& manifest_path
) {
    std::ifstream in(manifest_path);
    if (!in) {
        throw std::runtime_error("failed to open manifest: " + manifest_path.string());
    }

    const auto root = manifest_path.parent_path();

    std::vector<ManifestProblem> problems;
    std::string line;
    std::size_t line_no = 0;

    while (std::getline(in, line)) {
        ++line_no;

        if (line.empty()) {
            continue;
        }

        json j;
        try {
            j = json::parse(line);

            ManifestProblem problem;
            problem.id = j.at("id").get<std::string>();
            problem.map = root / j.at("map").get<std::string>();

            for (const auto& scenario : j.at("scenarios")) {
                problem.scenarios.emplace_back(root / scenario.get<std::string>());
            }

            problems.push_back(std::move(problem));
        } catch (const std::exception& e) {
            throw std::runtime_error(
                "invalid manifest line " + std::to_string(line_no) +
                ": " + e.what()
            );
        }
    }

    return problems;
}