module;

#include <optional>
module domain.songs;


using namespace Domain::Songs;

Services::SongService::SongService(DAOs::SongDAO& song_dao):
    song_dao(song_dao) {}

std::optional<Entities::Song::DTO> Services::SongService::retrieve(int id) {
    auto result = song_dao.retrieve(id);
    return result;
}

Entities::Song::DTO Services::SongService::create(const Entities::Song::CreatePayload& payload) {
    Entities::Song::DTO result = song_dao.create(payload);
    return result;
}

Entities::Song::DTO Services::SongService::update(int id, const Entities::Song::UpdatePayload& payload) {
    Entities::Song::DTO result = song_dao.update(id, payload);
    return result;
}
