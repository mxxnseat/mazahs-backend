module;

#include <samplerate.h>
#include <iostream>
#include <format>



module searcher.songs.dsp;

import core.config;

Searcher::Songs::DSP::ResampleNode::ResampleNode() : 
    target_samplerate(Core::Config::WORKING_SAMPLERATE()) {}

std::vector<float> Searcher::Songs::DSP::ResampleNode::process(Searcher::Songs::DSP::ReadFileOutput read_file_output) {
    if(read_file_output.samplerate == target_samplerate){
        std::cout << "Samplerate matches\nSkipping..." << std::endl;
        return read_file_output.data;
    }

    const double ratio = static_cast<double>(target_samplerate) / read_file_output.samplerate;

    const long input_frames = static_cast<long>(read_file_output.data.size());
    const long output_frames = static_cast<long>(input_frames * ratio) + 64;

    std::vector<float> output(static_cast<size_t>(output_frames));

    SRC_DATA src_data{};
    src_data.data_in = read_file_output.data.data();
    src_data.input_frames = input_frames;

    src_data.data_out = output.data();
    src_data.output_frames = output_frames;

    src_data.src_ratio = ratio;
    src_data.end_of_input = 1;

    int err = src_simple(&src_data, SRC_SINC_FASTEST, 1);
    if (err != 0) {
        throw std::runtime_error(
            std::string("Resample error: ") + src_strerror(err)
        );
    }

    output.resize(static_cast<size_t>(src_data.output_frames_gen));
    std::cout << std::format("Successfully convert {} to {}", read_file_output.samplerate, target_samplerate) << std::endl;
    return output;
}