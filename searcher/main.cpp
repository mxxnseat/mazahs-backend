#include <cstdlib>
#include <hiredis/hiredis.h>
#include <kangaru/kangaru.hpp>


import core.aws;
import core.aws.config;
import core.ioc;
import core.config;
import config;
import core.ytdlp;
import searcher.songs;
import searcher.songs.ioc;
import redis;
import std;
import jobs;
import postgres;


int main(){
    kgr::container container;

    Postgres& postgres = container.service<Core::IoC::PostgresService>();
    postgres.connect(Core::Config::DATABASE_URL());
    container.service<Core::IoC::RedisClientService>();
    Jobs::Server& server = container.service<Core::IoC::JobsServerService>();
    Searcher::Songs::SongsQueue& songs_queue = container.service<Searcher::Songs::IoC::SongsQueueService>();
    Searcher::Songs::SongsHashesQueue& songs_hashes_queue = container.service<Searcher::Songs::IoC::SongsHashesQueueService>();
    container.service<Searcher::Songs::IoC::SongsPullHandlerService>();
    container.service<Searcher::Songs::IoC::SongsHashesExtractHandlerService>();
        
    server.register_queue(songs_queue);
    server.register_queue(songs_hashes_queue);

    server.run();
    return 0;
}