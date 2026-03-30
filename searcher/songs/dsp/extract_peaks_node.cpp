module;

#include <vector>
#include <Eigen/dense>
#include <iostream>

module searcher.songs.dsp;

import core.utils;

std::vector<Searcher::Songs::DSP::ExtractPeaksOutput> Searcher::Songs::DSP::ExtractPeaksNode::process(Eigen::MatrixXcf& spectrogram) {
    std::vector<Searcher::Songs::DSP::ExtractPeaksOutput> peaks;
    Eigen::MatrixXf spectrogram_magnitude = spectrogram.cwiseAbs();
    spectrogram_magnitude = 20.0f * (spectrogram_magnitude.array() + 1e-6f).log10();
    Eigen::MatrixXf max_filtered_magnitude = Core::Utils::max_filter(spectrogram_magnitude, 10);
    float threshold =  spectrogram_magnitude.maxCoeff() - 60;
    int time_rows = spectrogram.rows();
    int frequency_cols = spectrogram.cols();


    for (int time = 0; time < time_rows; time++) {
        std::vector<std::pair<int, float>> frame_peaks;
        for (int frequency = 0; frequency < frequency_cols; frequency++) {
            float original_spectrogram_magnitude = spectrogram_magnitude(time, frequency);
            if (original_spectrogram_magnitude == max_filtered_magnitude(time, frequency) && original_spectrogram_magnitude >= threshold) {
                frame_peaks.push_back({frequency, original_spectrogram_magnitude});
            }
        }   
        std::sort(
            frame_peaks.begin(),
            frame_peaks.end(),
            [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
                return a.second > b.second;
            }
        );

        int keep_count = std::min<int>(5, frame_peaks.size());
        for (int i = 0; i < keep_count; ++i) {
            peaks.push_back({
                .frequency = frame_peaks[i].first,
                .time = time
            });
        }
    }
    std::sort(
        peaks.begin(),
        peaks.end(),
        [](const Searcher::Songs::DSP::ExtractPeaksOutput& a, const Searcher::Songs::DSP::ExtractPeaksOutput& b) {
            return a.time < b.time;
        }
    );
    return peaks;
}

bool Searcher::Songs::DSP::ExtractPeaksNode::is_peak(float spectrogram_magnitude, float local_maxima){
    float db_threashold = 2.0;
    return spectrogram_magnitude >= db_threashold && spectrogram_magnitude >= local_maxima;
}