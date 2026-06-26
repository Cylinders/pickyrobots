#pragma once

#include <filesystem>

#include "onnxruntime_cxx_api.h"
#include "valuation.h"

struct MapfasterValuation final : Valuation<MapfasterPortfolio> {
public:
    explicit MapfasterValuation(const std::filesystem::path &model_path);

    [[nodiscard]] std::array<SolverWeight, 4> evaluate(const ValuationInput &input) override;

private:
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    std::unique_ptr<Ort::Session> session_;

    Ort::MemoryInfo memory_info_;

    std::string input_name_;
    std::string output_name_;

    std::array<const char *, 1> input_names_{};
    std::array<const char *, 1> output_names_{};
};
