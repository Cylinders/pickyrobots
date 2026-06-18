#pragma once
#include "mapfaster_encoder.h"
#include "onnxruntime_cxx_api.h"
#include "valuation.h"

struct MapfasterValuation final : Valuation<MapfasterPortfolio> {
public:
    explicit MapfasterValuation(const std::filesystem::path& model_path)
        : env_{ORT_LOGGING_LEVEL_WARNING, "mapfaster"},
          memory_info_{Ort::MemoryInfo::CreateCpu(
              OrtArenaAllocator,
              OrtMemTypeDefault
          )}
    {
        session_options_.SetIntraOpNumThreads(1);

        // todo: gpu not working
        // OrtCUDAProviderOptions cuda_options{};
        // cuda_options.device_id = 0;
        // session_options_.AppendExecutionProvider_CUDA(cuda_options);

        session_ = std::make_unique<Ort::Session>(
            env_,
            model_path.c_str(),
            session_options_
        );

        Ort::AllocatorWithDefaultOptions allocator;

        auto input_name = session_->GetInputNameAllocated(0, allocator);
        auto output_name = session_->GetOutputNameAllocated(0, allocator);

        input_name_ = input_name.get();
        output_name_ = output_name.get();

        input_names_[0] = input_name_.c_str();
        output_names_[0] = output_name_.c_str();
    }

    [[nodiscard]] std::array<SolverWeight, 4>
    evaluate(const ValuationInput& input) override {
        auto input_buffer =
            mapfaster_encode(input.grid, input.agents, input.paths);

        std::array<int64_t, 4> input_shape{
            1, 3, IMAGE_SIZE, IMAGE_SIZE
        };

        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info_,
            input_buffer.data(),
            input_buffer.size(),
            input_shape.data(),
            input_shape.size()
        );

        auto outputs = session_->Run(
            Ort::RunOptions{nullptr},
            input_names_.data(),
            &input_tensor,
            1,
            output_names_.data(),
            1
        );

        const float* y = outputs[0].GetTensorData<float>();

        std::array<SolverWeight, 4> result{};

        for (size_t i = 0; i < result.size(); ++i) {
            result[i] = SolverWeight{
                MapfasterPortfolio::solvers[i],
                static_cast<double>(y[i])
            };
        }

        return result;
    }

private:
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    std::unique_ptr<Ort::Session> session_;

    Ort::MemoryInfo memory_info_;

    std::string input_name_;
    std::string output_name_;

    std::array<const char*, 1> input_names_{};
    std::array<const char*, 1> output_names_{};
};