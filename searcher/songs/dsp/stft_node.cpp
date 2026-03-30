module;

#include <Eigen/dense>

module searcher.songs.dsp;

import core.utils;
import core.config;

Searcher::Songs::DSP::STFTNode::STFTNode() : 
    window_length(Core::Config::STFT_WINDOW_LENGTH()),
    hop_length(Core::Config::STFT_HOP_LENGTH()) {}

Eigen::MatrixXcf Searcher::Songs::DSP::STFTNode::process(const std::vector<float>& data) {
    return Core::Utils::stft(data, hop_length, window_length);
}