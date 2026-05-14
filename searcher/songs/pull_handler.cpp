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
import core.ffmpeg;

Searcher::Songs::SongsPullHandler::SongsPullHandler(
    Domain::Songs::Services::SongService& song_service,
    Core::AWS::S3Client& s3_client,
    Core::AWS::Config::S3Options& s3_options
) :
    Jobs::Handler<SongsPullPayload>(Searcher::Songs::Constants::SONGS_PULL_TASK_NAME),
    output_directory(Core::Config::DOWNLOAD_DIRECTORY()),
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
    Core::AWS::S3MultipartUploader uploader(
        s3_client, 
        s3_options.songs_bucket, 
        object_key
    );
    
    int ytdlp_result = ytdlp_commander
        .set_url(payload.url)
        .pipe(Core::FFMPEG::Commander()
            .set_format("ogg")
            .use_experemental()
            .set_channels_number(2)
            .set_codec("vorbis")
            .set_samplerate(Core::Config::WORKING_SAMPLERATE())
            .pipe(uploader)
        );

        if(ytdlp_result != 0){
        throw std::runtime_error("download failed");
    }

    Domain::Songs::Entities::Song::DTO song = song_service.create({
        .url = payload.url,
        .status = SONG_STATUS::to_string(SongStatus::Pending),
        .audio_file_path = object_key,
    });
    // songs_hashes_queue.add(Searcher::Songs::Constants::SONGS_HASHES_EXTRACT_TASK_NAME, extract_payload);
}