module;

#include <vector>

module searcher.songs.dsp;

std::vector<Searcher::Songs::DSP::BuildHashOutput> Searcher::Songs::DSP::BuildHashNode::process(std::vector<ExtractPeaksOutput>& peaks) {
    std::vector<BuildHashOutput> hashes;
    for (int i = 0; i < peaks.size(); i++) {
        int used = 0;
        for (int j = i + 1; j < peaks.size(); j++) {
            int first_time = peaks[i].time;
            int second_time = peaks[j].time;
            int delta_time = second_time - first_time;
            if(delta_time < 10){
                continue;
            }
            if(delta_time > 40){
               break;
            }
            uint32_t built_hash = hash(peaks[i].frequency, peaks[j].frequency, delta_time);
            hashes.push_back({.time = first_time, .hash = built_hash});

            used++;
            if(used >= 5){
                break;
            }
        }
    }
    return hashes;
}

uint32_t Searcher::Songs::DSP::BuildHashNode::hash(int frequency1, int frequency2, int delta_time) {
    return (frequency1 << 20) | (frequency2 << 10) | delta_time;
}