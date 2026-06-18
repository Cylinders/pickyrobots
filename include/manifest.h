#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct ManifestProblem {
    std::string id;
    std::filesystem::path map;
    std::vector<std::filesystem::path> scenarios;
};

std::vector<ManifestProblem> parse_manifest(
    const std::filesystem::path& manifest_path
);