module;

module searcher.songs;

import core.config;
import domain.songs;
import std;
import core.utils;


Searcher::Songs::SongsPullHandler::SongsPullHandler(SongsHashesQueue& songs_hashes_queue, Domain::Songs::Services::SongService& song_service) :
    Jobs::Handler<SongsPullPayload>(Searcher::Songs::Constants::SONGS_PULL_TASK_NAME),
    output_directory(Core::Config::DOWNLOAD_DIRECTORY()),
    songs_hashes_queue(songs_hashes_queue),
    song_service(song_service) {}

void Searcher::Songs::SongsPullHandler::process(const SongsPullPayload& payload) const {
    using namespace Domain::Songs::Entities;

    Core::Utils::YtdlpResponse ytdlp_result = Core::Utils::run_ytdlp(payload.url, output_directory);
    if(ytdlp_result.status != 0){
        throw std::runtime_error("download failed");
    }

    Domain::Songs::Entities::Song::DTO song = song_service.create({
        .name = "test",
        .url = payload.url,
        .status = SONG_STATUS::to_string(SongStatus::Pending)
    });

    SongsHashesExtractPayload extract_payload = {
        .path = *ytdlp_result.file_path,
        .song_id = song.id
    };

    songs_hashes_queue.add(Searcher::Songs::Constants::SONGS_HASHES_EXTRACT_TASK_NAME, extract_payload);
}