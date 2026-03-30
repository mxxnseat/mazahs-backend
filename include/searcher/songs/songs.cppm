module;

#include <nlohmann/json.hpp>

export module searcher.songs;

import std;
import jobs;
import redis;
import domain.songs;
import searcher.songs.dsp;

export namespace Searcher::Songs {
    namespace Constants {
        inline const constexpr char* SONGS_PULL_TASK_NAME = "songs:pull";
        inline const constexpr char* SONGS_HASHES_EXTRACT_TASK_NAME = "songs_hashes:extract";
    }

    struct SongsPullPayload {
        std::string url;
    };

    struct SongsHashesExtractPayload {
        std::string path;
        int song_id;
    };

    void to_json(nlohmann::json& j, const SongsPullPayload& payload);
    void from_json(const nlohmann::json& j, SongsPullPayload& payload);
    
    void to_json(nlohmann::json& j, const SongsHashesExtractPayload& payload);
    void from_json(const nlohmann::json& j, SongsHashesExtractPayload& payload);

    class SongsQueue : public Jobs::Queue<SongsPullPayload> {
        public:
            SongsQueue(const RedisClient& redis);
    };

    class SongsHashesQueue : public Jobs::Queue<SongsHashesExtractPayload> {
        public:
            SongsHashesQueue(const RedisClient& redis);
    };

    class SongsPullHandler : public Jobs::Handler<SongsPullPayload> {
        public:
            SongsPullHandler(SongsHashesQueue& songs_hashes_queue, Domain::Songs::Services::SongService& song_service);
            void process(const SongsPullPayload& payload) const override;

        private:
            std::string output_directory;
            SongsHashesQueue& songs_hashes_queue;
            Domain::Songs::Services::SongService& song_service;
    };

    class SongsHashesExtractHandler : public Jobs::Handler<SongsHashesExtractPayload> {
        public:
            SongsHashesExtractHandler(
                Searcher::Songs::DSP::ReadFileNode& read_file_node,
                Searcher::Songs::DSP::ResampleNode& resample_node,
                Searcher::Songs::DSP::STFTNode& stft_node,
                Searcher::Songs::DSP::ExtractPeaksNode& extract_peaks_node,
                Searcher::Songs::DSP::BuildHashNode& build_hash_node,
                Domain::Songs::Services::SongHashService& song_hash_service,
                Domain::Songs::Services::SongService& song_service
            );
            void process(const SongsHashesExtractPayload& payload) const override;

        private:
            Searcher::Songs::DSP::ReadFileNode& read_file_node;
            Searcher::Songs::DSP::ResampleNode& resample_node;
            Searcher::Songs::DSP::STFTNode& stft_node;
            Searcher::Songs::DSP::ExtractPeaksNode& extract_peaks_node;
            Searcher::Songs::DSP::BuildHashNode& build_hash_node;
            Domain::Songs::Services::SongHashService& song_hash_service;
            Domain::Songs::Services::SongService& song_service;
    };
}