module;

#include "kangaru/service.hpp"
#include <kangaru/kangaru.hpp>

export module searcher.songs.ioc;

import core.ioc;
import searcher.songs;
import searcher.songs.dsp;
import domain.songs.ioc;

using namespace Core::IoC::Utils;

export namespace Searcher::Songs::IoC {
    // DSP
    struct ReadFileNodeService : kgr::single_service<Searcher::Songs::DSP::ReadFileNode> {};
    struct ResampleNodeService : kgr::single_service<Searcher::Songs::DSP::ResampleNode> {};
    struct STFTNodeService : kgr::single_service<Searcher::Songs::DSP::STFTNode> {};
    struct ExtractPeaksNodeService : kgr::single_service<Searcher::Songs::DSP::ExtractPeaksNode> {};
    struct BuildHashNodeService : kgr::single_service<Searcher::Songs::DSP::BuildHashNode> {};
    struct PeakNormalizationNodeService : kgr::single_service<Searcher::Songs::DSP::PeakNormalizationNode> {};
    struct BandpassFilterNodeService : kgr::single_service<Searcher::Songs::DSP::BandpassFilterNode> {};

    // Other
    struct SongsQueueService : kgr::single_service<Searcher::Songs::SongsQueue, kgr::dependency<Core::IoC::RedisClientService>> {};
    struct SongsHashesQueueService : kgr::single_service<Searcher::Songs::SongsHashesQueue, kgr::dependency<Core::IoC::RedisClientService>> {};

    struct SongsPullHandlerService : 
        kgr::single_service<Searcher::Songs::SongsPullHandler, kgr::dependency<SongsHashesQueueService, Domain::Songs::IoC::SongServiceService>>, 
        kgr::autocall<method<&Searcher::Songs::SongsPullHandler::bootstrap>> {};
    struct SongsHashesExtractHandlerService:
        kgr::single_service<Searcher::Songs::SongsHashesExtractHandler, 
            kgr::dependency<
                ReadFileNodeService,
                ResampleNodeService, 
                STFTNodeService, 
                ExtractPeaksNodeService, 
                BuildHashNodeService,
                Domain::Songs::IoC::SongHashServiceService,
                Domain::Songs::IoC::SongServiceService
            >
        >,
        kgr::autocall<method<&Searcher::Songs::SongsHashesExtractHandler::bootstrap>> {};

}