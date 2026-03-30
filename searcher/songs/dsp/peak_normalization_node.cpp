module;

#include <vector>

module searcher.songs.dsp;


std::vector<float> Searcher::Songs::DSP::PeakNormalizationNode::process(std::vector<float> data) {
    float peak = 0.0f;

    for (float x : data) {
        peak = std::max(peak, std::abs(x));
    }
    if (peak <= 0.0f) {
        return data;
    }
    const float gain = target_peak / peak;
    for (float& x : data) {
        x *= gain;
    }
    return data;
}