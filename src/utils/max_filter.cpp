module;

#include <fftw3.h>
#include <Eigen/Dense>
#include <cstdlib>
#include <limits>
#include <algorithm>

module core.utils;

// TODO: Refactore to use O(N) -> Use 1d max filter for columns and rows
Eigen::MatrixXf Core::Utils::max_filter(const Eigen::MatrixXf& spectrogram, int window_size) {
    Eigen::MatrixXf result = Eigen::MatrixXf::Zero(spectrogram.rows(), spectrogram.cols());

    const int rows = spectrogram.rows();
    const int cols = spectrogram.cols();

    const int before = (window_size - 1) / 2;
    const int after = window_size - before;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            float max_val = std::numeric_limits<float>::lowest();

            for (int r = row - before; r < row + after; ++r) {
                for (int c = col - before; c < col + after; ++c) {
                    if (r >= 0 && r < rows && c >= 0 && c < cols) {
                        max_val = std::max(max_val, spectrogram(r, c));
                    }
                }
            }

            result(row, col) = max_val;
        }
    }

    return result;
}
