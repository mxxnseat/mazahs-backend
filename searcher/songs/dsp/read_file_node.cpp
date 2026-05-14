module;

#include <string>
#include <sndfile.h>
#include <format>
#include <iostream>
#include <iomanip>

module searcher.songs.dsp;

import core.aws;

using namespace Searcher::Songs::DSP;
using namespace Core::AWS;

SF_VIRTUAL_IO create_s3_virtual_io();

struct VioContext {
    S3ObjectReader& reader;
    sf_count_t total_size;
};


ReadFileNode::ReadFileNode(Core::AWS::S3Client& s3_client, Core::AWS::Config::S3Options& s3_options) :
    s3_client(s3_client),
    s3_options(s3_options) {}

    ReadFileOutput ReadFileNode::process(std::string path) {
    S3ObjectReader reader(s3_client, s3_options.songs_bucket, path);
    SF_VIRTUAL_IO vio = create_s3_virtual_io();
    SF_INFO info{};
    SNDFILE* file = sf_open_virtual(&vio, SFM_READ, &info, &reader);
    if (!file) {
        const char* err = sf_strerror(nullptr);
        throw std::runtime_error("Failed to open virtual file '" + path + "': " + err);
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

SF_VIRTUAL_IO create_s3_virtual_io() {
    return {
        .get_filelen = [](void* ud) -> sf_count_t {
            return static_cast<S3ObjectReader*>(ud)->get_total_size();
        },
        .seek = [](sf_count_t offset, int whence, void* ud) -> sf_count_t {
            auto* reader = static_cast<S3ObjectReader*>(ud);
            sf_count_t new_pos;
            switch (whence) {
                case SEEK_SET: new_pos = offset; break;
                case SEEK_CUR: new_pos = reader->tell() + offset; break;
                case SEEK_END: new_pos = reader->get_total_size() + offset; break;
            }
            return reader->seek(new_pos);
        },
        .read = [](void* dest, sf_count_t count, void* ud) -> sf_count_t {
            return static_cast<S3ObjectReader*>(ud)->read(dest, count);
        },
        .write = nullptr,
        .tell = [](void* ud) -> sf_count_t {
            return static_cast<S3ObjectReader*>(ud)->tell();
        },
    };
}
