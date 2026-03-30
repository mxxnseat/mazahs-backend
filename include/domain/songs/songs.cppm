module;

#include <pqxx/pqxx>
#include <unordered_map>
#include <vector>

export module domain.songs;

import searcher.songs.dsp;
import std;
import postgres;

export namespace Domain::Songs {
    namespace Entities {
        enum class SongStatus {
            Pending,
            Ready
        };

        struct SONG_STATUS {
            static std::string to_string(SongStatus status) {
                switch(status){
                    case SongStatus::Pending:
                        return "pending";
                    case SongStatus::Ready:
                        return "ready";
                }
            }

            static SongStatus from_string(const std::string& status){
                if(status == "pending"){
                    return SongStatus::Pending;
                }
                if(status == "ready"){
                    return SongStatus::Ready;
                }
            }
        };

        struct Song {
            struct DTO {
                int id;
                std::string name;
                std::string url;
                std::string status;

                static Song::DTO from_row(const pqxx::row& result) {
                    return Song::DTO{
                        .id = result["id"].as<int>(),
                        .name = result["name"].as<std::string>(),
                        .url = result["url"].as<std::string>(),
                        .status = result["status"].as<std::string>(),
                    };
                }
            };

            struct CreatePayload {
                std::string name;
                std::string url;
                std::string status;

                static std::unordered_map<std::string, std::string> to_fields(const CreatePayload& payload) {
                    return {
                        {"name", payload.name},
                        {"url", payload.url},
                        {"status", payload.status}
                    };
                }
            };

            struct UpdatePayload {
                std::optional<std::string> name;
                std::optional<std::string> url;
                std::optional<std::string> status;

                static std::unordered_map<std::string, std::string> to_fields(const UpdatePayload& payload) {
                    std::unordered_map<std::string, std::string> result;
                    if(payload.name){
                        result.emplace("name", *payload.name);
                    }
                    if(payload.url){
                        result.emplace("url", *payload.url);
                    }
                    if(payload.status){
                        result.emplace("status", *payload.status);
                    }
                    return result;
                }
            };
        };

        struct SongHash {
            struct DTO {
                int id;
                int song_id;
                int anchor_time;
                int hash;

                static SongHash::DTO from_row(const pqxx::row& result) {
                    return SongHash::DTO{
                        .id = result["id"].as<int>(),
                        .song_id = result["song_id"].as<int>(),
                        .anchor_time = result["anchor_time"].as<int>(),
                        .hash = result["hash"].as<int>(),
                    };
                }
            };

            struct VoteResult {
                std::string url;
                int anchor_time;

                static SongHash::VoteResult from_row(const pqxx::row& result) {
                    return SongHash::VoteResult{
                        .url = result["url"].as<std::string>(),
                        .anchor_time = result["anchor_time"].as<int>(),
                    };
                }
            };

            struct CreatePayload {
                int song_id;
                int anchor_time;
                int hash;

                static std::unordered_map<std::string, int> to_fields(const CreatePayload& payload) {
                    return {
                        {"song_id", payload.song_id},
                        {"anchor_time", payload.anchor_time},
                        {"hash", payload.hash}
                    };
                }
            };
        };
    };

    namespace DAOs{
        class SongDAO {
            public:
                SongDAO(const Postgres& postgres);

                Entities::Song::DTO create(const Entities::Song::CreatePayload& payload);
                Entities::Song::DTO update(int id, const Entities::Song::UpdatePayload& payload);
            private:
                const Postgres& postgres;
                std::string table;
        };

        class SongHashDAO {
            public:
                SongHashDAO(const Postgres& postgres);

                std::vector<Entities::SongHash::DTO> create_many(const std::vector<Entities::SongHash::CreatePayload>& payload);
                std::optional<Entities::SongHash::VoteResult> vote(const std::vector<Searcher::Songs::DSP::BuildHashOutput>& payload);
            private:
                const Postgres& postgres;
                std::string table;
        };
    }

    namespace Services {
        class SongService {
            public:
                SongService(DAOs::SongDAO& song_dao);

                Entities::Song::DTO create(const Entities::Song::CreatePayload& payload);
                Entities::Song::DTO update(int id, const Entities::Song::UpdatePayload& payload);
            private:
                DAOs::SongDAO& song_dao;
        };

        class SongHashService {
            public:
                SongHashService(DAOs::SongHashDAO& song_hash_dao);

                std::vector<Entities::SongHash::DTO> create_many(const std::vector<Entities::SongHash::CreatePayload>& payload);
                std::optional<Entities::SongHash::VoteResult> vote(const std::vector<Searcher::Songs::DSP::BuildHashOutput>& payload);
            private:
                DAOs::SongHashDAO& song_hash_dao;
        };
    }
};
