#include <span>
#include <vector>
#include <opencv2/imgcodecs.hpp>

#include "../include/mapfaster_encoder.h"

inline std::vector<float> preprocess_rgb_one(const cv::Mat &rgb) {
    if (rgb.dims != 2 || rgb.channels() != 3 || rgb.type() != CV_8UC3) {
        throw std::invalid_argument("expected HxWx3 uint8 RGB image");
    }

    constexpr int C = 3;
    constexpr int H = IMAGE_SIZE;
    constexpr int W = IMAGE_SIZE;

    const int h = rgb.rows;
    const int w = rgb.cols;

    const double scale = static_cast<double>(IMAGE_SIZE) / std::max(h, w);
    const int new_w = static_cast<int>(std::round(w * scale));
    const int new_h = static_cast<int>(std::round(h * scale));

    cv::Mat resized;
    cv::resize(rgb, resized, cv::Size(new_w, new_h), 0, 0, cv::INTER_NEAREST);

    cv::Mat padded(
        IMAGE_SIZE,
        IMAGE_SIZE,
        CV_8UC3,
        cv::Scalar(0, 0, 0)
    );

    const int top = (IMAGE_SIZE - new_h) / 2;
    const int left = (IMAGE_SIZE - new_w) / 2;

    resized.copyTo(padded(cv::Rect(left, top, new_w, new_h)));

    std::vector<float> out(C * H * W);

    constexpr std::array mean = {0.485f, 0.456f, 0.406f};
    constexpr std::array std = {0.229f, 0.224f, 0.225f};

    for (int y = 0; y < H; ++y) {
        const auto *row = padded.ptr<cv::Vec3b>(y);

        for (int x = 0; x < W; ++x) {
            const cv::Vec3b px = row[x];

            for (int c = 0; c < C; ++c) {

                const float v = static_cast<float>(px[c]) / 255.0f;
                out[c * H * W + y * W + x] = (v - mean[c]) / std[c];
            }
        }
    }

    return out;
}

const cv::Vec3b FREE{255, 255, 255}; // white
const cv::Vec3b OBSTACLE{255, 0, 0}; // red
const cv::Vec3b START{0, 255, 0}; // green
const cv::Vec3b GOAL{0, 0, 255}; // blue
const cv::Vec3b PATH{0, 0, 0}; // black

std::vector<float> mapfaster_encode(const Grid &grid, const std::vector<Agent> &agents,
                                    const std::unordered_map<Agent, std::vector<Pos> > &paths) {
    cv::Mat image(grid.height, grid.width, CV_8UC3);

    for (int row = 0; row < grid.height; ++row) {
        for (int col = 0; col < grid.width; ++col) {
            image.at<cv::Vec3b>(row, col) =
                    grid.is_blocked(col, row) ? OBSTACLE : FREE;
        }
    }

    for (const auto &agent: agents) {
        for (const auto &path = paths.at(agent); const auto &[row, col]: path) {
            image.at<cv::Vec3b>(row, col) = PATH;
        }

        image.at<cv::Vec3b>(agent.start.row, agent.start.col) = START;
        image.at<cv::Vec3b>(agent.goal.row, agent.goal.col) = GOAL;
    }

    cv::Mat bgr;
    cv::cvtColor(image, bgr, cv::COLOR_RGB2BGR);

    cv::imwrite(
        "../dump/image_n.png",
        bgr
    );

    return preprocess_rgb_one(image);
}
