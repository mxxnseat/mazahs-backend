module;

#include "Eigen/Core"
#include <Eigen/dense>
#include <cmath>

module searcher.songs;

import core.config;
import domain.songs;
import std;
import core.utils;

using namespace Domain::Songs;

Searcher::Songs::SongsHashesExtractHandler::SongsHashesExtractHandler(
        Searcher::Songs::DSP::ReadFileNode& read_file_node,
        Searcher::Songs::DSP::ResampleNode& resample_node,
        Searcher::Songs::DSP::STFTNode& stft_node,
        Searcher::Songs::DSP::ExtractPeaksNode& extract_peaks_node,
        Searcher::Songs::DSP::BuildHashNode& build_hash_node,
        Services::SongHashService& song_hash_service,
        Services::SongService& song_service
    ) :
    read_file_node(read_file_node),
    resample_node(resample_node),
    stft_node(stft_node),
    extract_peaks_node(extract_peaks_node),
    build_hash_node(build_hash_node),
    song_hash_service(song_hash_service),
    song_service(song_service),
    Jobs::Handler<SongsHashesExtractPayload>(Searcher::Songs::Constants::SONGS_HASHES_EXTRACT_TASK_NAME) {}

void Searcher::Songs::SongsHashesExtractHandler::process(const SongsHashesExtractPayload& payload) const {
    DSP::ReadFileOutput read_file_output = read_file_node.process(payload.path);
    std::vector<float> resampled_data = resample_node.process(read_file_output);
    Eigen::MatrixXcf spectrogram = stft_node.process(resampled_data);
    std::vector<Searcher::Songs::DSP::ExtractPeaksOutput> peaks = extract_peaks_node.process(spectrogram);
    std::vector<Searcher::Songs::DSP::BuildHashOutput> hashes = build_hash_node.process(peaks);
    
    // Creating batch
    constexpr int batch_size = 5'000;
    int reserve_size = static_cast<int>(std::ceil(hashes.size() / batch_size));
    
    std::vector<std::vector<Entities::SongHash::CreatePayload>> batches;
    batches.reserve(reserve_size);
    batches.emplace_back();
    batches.back().reserve(batch_size);

    for(int i = 0;i<hashes.size();i++) {
        const Searcher::Songs::DSP::BuildHashOutput& hash_result = hashes[i];
        batches.back().push_back({
            .song_id = payload.song_id,
            .anchor_time = hash_result.time,
            .hash = static_cast<int>(hash_result.hash)
        });

        if(batches.back().size() == batch_size && i < hashes.size() - 1){
                batches.emplace_back();
                batches.back().reserve(batch_size);
        }
    }

    for(const auto& batch : batches) {
        song_hash_service.create_many(batch);
    }
    song_service.update(payload.song_id, {.status = Entities::SONG_STATUS::to_string(Entities::SongStatus::Ready)});
}