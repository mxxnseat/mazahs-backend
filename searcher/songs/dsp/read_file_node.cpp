module;

#include <string>
#include <sndfile.h>
#include <format>

module searcher.songs.dsp;


Searcher::Songs::DSP::ReadFileOutput Searcher::Songs::DSP::ReadFileNode::process(std::string path) {
    SF_INFO info;
    SNDFILE *file = sf_open(path.c_str(), SFM_READ, &info);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::vector<float> data(info.frames*info.channels);
    sf_count_t frames_read = sf_readf_float(file, data.data(), info.frames);
    if (frames_read != info.frames) {
        throw std::runtime_error(
            std::format("Failed to read file: {} ({})", path, sf_strerror(file))
        );
    }

    sf_close(file);

    std::vector<float> mono_data(frames_read);
    for (int i = 0; i < frames_read; i++) {
        mono_data[i] = data[i * info.channels];
    }
    return {
        .data = mono_data,
        .samplerate = info.samplerate
    };
}