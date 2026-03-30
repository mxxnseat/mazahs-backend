module;

#include <kangaru/kangaru.hpp>

export module websocket.songs.ioc;

import searcher.songs.ioc;
import domain.songs.ioc;
import websocket.songs;

export namespace Websocket::Songs::IoC {
    struct SongsWebsocketMethodService : kgr::single_service<Websocket::Songs::SongsWebsocketMethod, 
        kgr::dependency<
                Searcher::Songs::IoC::ResampleNodeService, 
                Searcher::Songs::IoC::STFTNodeService, 
                Searcher::Songs::IoC::ExtractPeaksNodeService, 
                Searcher::Songs::IoC::BuildHashNodeService,
                Searcher::Songs::IoC::PeakNormalizationNodeService,
                Searcher::Songs::IoC::BandpassFilterNodeService,
                Domain::Songs::IoC::SongHashServiceService,
                Domain::Songs::IoC::SongServiceService
            >
        > {};
}