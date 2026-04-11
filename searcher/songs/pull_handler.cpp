module;

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

module searcher.songs;

import core.aws.config;
import core.config;
import domain.songs;
import std;
import core.utils;
import core.aws;
import core.ytdlp;


Searcher::Songs::SongsPullHandler::SongsPullHandler(
    SongsHashesQueue& songs_hashes_queue, 
    Domain::Songs::Services::SongService& song_service,
    Core::AWS::S3Client& s3_client,
    Core::AWS::Config::S3Options& s3_options
) :
    Jobs::Handler<SongsPullPayload>(Searcher::Songs::Constants::SONGS_PULL_TASK_NAME),
    output_directory(Core::Config::DOWNLOAD_DIRECTORY()),
    songs_hashes_queue(songs_hashes_queue),
    song_service(song_service),
    s3_client(s3_client),
    s3_options(s3_options)
    {}

void Searcher::Songs::SongsPullHandler::process(const SongsPullPayload& payload) const {
    using namespace Domain::Songs::Entities;

    boost::uuids::random_generator gen;
    boost::uuids::uuid uuid = gen();

    std::string object_key = "wav/" + boost::uuids::to_string(uuid);

    Core::Ytdlp::Commander ytdlp_commander;
    
    int ytdlp_result = ytdlp_commander
        .set_url(payload.url)
        .set_audio_format("wav")
        .pipe(Core::AWS::S3MultipartUploader(s3_client, s3_options.songs_bucket, object_key));

    if(ytdlp_result != 0){
        throw std::runtime_error("download failed");
    }

    // TODO: Publish with debezium
    Domain::Songs::Entities::Song::DTO song = song_service.create({
        .name = "test",
        .url = payload.url,
        .status = SONG_STATUS::to_string(SongStatus::Pending),
        .audio_file_path = object_key,
    });

    // SongsHashesExtractPayload extract_payload = {
    //     .path = *ytdlp_result.file_path,
    //     .song_id = song.id
    // };

    // songs_hashes_queue.add(Searcher::Songs::Constants::SONGS_HASHES_EXTRACT_TASK_NAME, extract_payload);
}