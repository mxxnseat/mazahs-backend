module;

#include <vector>

module domain.songs;

import searcher.songs.dsp;


using namespace Domain::Songs;

Services::SongHashService::SongHashService(DAOs::SongHashDAO& song_hash_dao):
    song_hash_dao(song_hash_dao) {}

std::vector<Entities::SongHash::DTO> Services::SongHashService::create_many(const std::vector<Entities::SongHash::CreatePayload>& payload) {
    std::vector<Entities::SongHash::DTO> result = song_hash_dao.create_many(payload);
    return result;
}


std::optional<Entities::SongHash::VoteResult> Services::SongHashService::vote(const std::vector<Searcher::Songs::DSP::BuildHashOutput>& payload) {
    std::optional<Entities::SongHash::VoteResult> result = song_hash_dao.vote(payload);
    return result;
}
