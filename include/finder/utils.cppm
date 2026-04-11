module;

#include <Eigen/Dense>
#include <vector>

export module core.utils;

import std;

export namespace Core::Utils {
    std::vector<float> hann_window(int N);
    Eigen::MatrixXcf stft(const std::vector<float> &data, int hop_size, int fft_size);
    Eigen::MatrixXf max_filter(const Eigen::MatrixXf &spectrogram, int window_size);
    std::vector<std::string> split(const std::string& str, char delimiter);
}
