module;

#include <string>
#include <iostream>
#include <nlohmann/json_fwd.hpp>

export module searcher.songs.amqp;

import searcher.songs;
import core.amqp;

export namespace Searcher::Songs::AMQP::Handlers {
    using namespace Core::AMQP;
    using namespace Searcher::Songs;

    struct SongsClassifierPayload {
        int song_id;
        std::string kind;
    };

    class SongsClassifierHandler : public AmqpConsumerHandler<SongsClassifierPayload> {
        public:
            SongsClassifierHandler(SongsHashesQueue& songs_hashes_queue) : 
                songs_hashes_queue(songs_hashes_queue) 
            {}

            int process(const SongsClassifierPayload& payload) override {
                if(is_it_music(payload)){
                    auto extract_payload = SongsHashesExtractPayload{
                        .song_id = payload.song_id
                    };
                    songs_hashes_queue.add(Constants::SONGS_HASHES_EXTRACT_TASK_NAME, extract_payload);
                    return 0;
                }

                // TODO: Add more proper logging and user notification
                std::cout << "Not a song " << payload.song_id << std::endl;

                return 0;
            }

        private:
            SongsHashesQueue& songs_hashes_queue;

            bool is_it_music(const SongsClassifierPayload& payload) {
                return payload.kind == "music";
            }
    };

    void from_json(const nlohmann::json& j, SongsClassifierPayload& payload) {
        const nlohmann::json& payload_json = j.at("payload");
        payload_json.at("song_id").get_to(payload.song_id);
        payload_json.at("kind").get_to(payload.kind);
    }
}