module;

#include <nlohmann/json.hpp>

module searcher.songs;

import std;

void Searcher::Songs::to_json(nlohmann::json& j, const SongsPullPayload& payload) {
    j = nlohmann::json{{"url", payload.url}};
}

void Searcher::Songs::from_json(const nlohmann::json& j, SongsPullPayload& payload) {
    j.at("url").get_to(payload.url);
}

void Searcher::Songs::to_json(nlohmann::json& j, const SongsHashesExtractPayload& payload) {
    j = nlohmann::json{
        {"path", payload.path},
        {"song_id", payload.song_id}
    };
}

void Searcher::Songs::from_json(const nlohmann::json& j, SongsHashesExtractPayload& payload) {
    j.at("path").get_to(payload.path);
    j.at("song_id").get_to(payload.song_id);
}
