#include "kangaru/container.hpp"
#include <cstdlib>
#include <hiredis/hiredis.h>
#include <kangaru/kangaru.hpp>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <ev.h>
#include <thread>
#include <format>

import core.amqp.config;
import core.aws;
import core.aws.config;
import core.ioc;
import core.config;
import config;
import core.ytdlp;
import searcher.songs;
import searcher.songs.ioc;
import searcher.songs.amqp;
import redis;
import std;
import jobs;
import postgres;


void amqpConsumer(kgr::container& container) {
    auto& amqp_consumer = container.service<Core::IoC::AmqpConsumerService>();
    auto& amqp_context = container.service<Core::IoC::AmqpContextService>();
    auto& songs_classifier_handler = container.service<Searcher::Songs::IoC::SongsClassifierHandlerService>();

    amqp_consumer.subscribe<Searcher::Songs::AMQP::Handlers::SongsClassifierPayload>("songs-classifier", songs_classifier_handler);


    amqp_context.start();

    // auto *loop = EV_DEFAULT;

    // AMQP::LibEvHandler handler(loop);

    // AMQP::Address address(
    //     std::format("amqp://{}:{}@{}:{}/{}", 
    //         amqp_config.username.value(), 
    //         amqp_config.password.value(), 
    //         amqp_config.host, 
    //         amqp_config.port,
    //         amqp_config.vhost
    //     )
    // );
    // AMQP::TcpConnection connection(&handler, address);

    // AMQP::TcpChannel channel(&connection);

    // // ACK needs channel access
    // channel.consume("songs-classifier")
    //     .onSuccess([](const std::string &tag) {
            
    //         // the consumer was successfully started
    //         std::cout << "consumer " << tag << " started" << std::endl;

    //     })
    //     .onCancelled([](const std::string &tag) {
            
    //         // the consumer was cancelled by the server
    //         std::cout << "consumer " << tag << " was cancelled" << std::endl;

    //     })
    //     .onReceived([&channel](const AMQP::Message &msg,
    //                            uint64_t deliveryTag,
    //                            bool redelivered)
    //     {
    //         std::string body(msg.body(), msg.bodySize());
    //         std::cout << "Received: " << body << std::endl;

    //         channel.ack(deliveryTag);
    //     })
    //     .onError([](const char *message) {
            
    //         // there was an error starting the consumer
    //         std::cerr << "error starting consumer: " << message << std::endl;

    //     });

}

int main() {
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

    std::thread amqp_thread(amqpConsumer, std::ref(container));
    std::thread jobs_thread([&server]{
        server.run();
    });

    if(amqp_thread.joinable()){
        amqp_thread.join();
    }else {
        std::cout << "amqp thread could not be ran" << std::endl;
    }

    if(jobs_thread.joinable()){
        jobs_thread.join();
    }else {
        std::cout << "jobs thread could not be ran" << std::endl;
    }

    return 0;
}