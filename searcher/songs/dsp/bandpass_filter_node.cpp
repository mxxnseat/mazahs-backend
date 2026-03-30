module;

#include <vector>

module searcher.songs.dsp;

import core.config;


Searcher::Songs::DSP::BandpassFilterNode::BandpassFilterNode() {
    filter.setup(Core::Config::WORKING_SAMPLERATE(), float((80+8000)/2), float(8000-80));
}

std::vector<float> Searcher::Songs::DSP::BandpassFilterNode::process(std::vector<float> data) {
    for(float& x : data){
        x = filter.filter(x);
    }
    return data;
}