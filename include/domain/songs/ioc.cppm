module;

#include <kangaru/kangaru.hpp>

export module domain.songs.ioc;

import domain.songs;
import core.ioc;

using namespace Domain::Songs;

export namespace Domain::Songs::IoC {
    struct SongDAOService : kgr::single_service<DAOs::SongDAO, kgr::dependency<Core::IoC::PostgresService>> {};
    struct SongServiceService : kgr::single_service<Services::SongService, kgr::dependency<SongDAOService>> {};

    struct SongHashDAOService : kgr::single_service<DAOs::SongHashDAO, kgr::dependency<Core::IoC::PostgresService>> {};
    struct SongHashServiceService : kgr::single_service<Services::SongHashService, kgr::dependency<SongHashDAOService>> {};
}