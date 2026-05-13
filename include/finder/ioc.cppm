module;

#include "kangaru/service.hpp"
#include <kangaru/kangaru.hpp>

export module core.ioc;

import core.amqp;
import core.amqp.config;
import core.aws.config;
import core.aws;
import core.kafka.config;
import core.kafka;
import config;
import redis;
import postgres;
import jobs;
import core.websocket_server;

export namespace Core::IoC {
    namespace Utils {
        template<auto m>
        using method = kgr::method<decltype(m), m>;
    }

    struct ConfigServiceService : kgr::single_service<ConfigService> {};
    struct PostgresService : kgr::single_service<Postgres> {};
    struct RedisClientService : kgr::single_service<RedisClient>{};
    struct JobsServerService : kgr::single_service<Jobs::Server, kgr::dependency<RedisClientService>> {};
    
    struct KafkaProducerOptionsService
        : kgr::single_service<
            Core::Kafka::Config::KafkaProducerOptions,
            kgr::dependency<ConfigServiceService>> {};
    struct KafkaProducerService : kgr::single_service<Core::Kafka::KafkaProducer, kgr::dependency<KafkaProducerOptionsService>> {};

    struct S3OptionsService
        : kgr::single_service<
            Core::AWS::Config::S3Options,
            kgr::dependency<ConfigServiceService>> {};
    struct S3ClientService : kgr::single_service<Core::AWS::S3Client, kgr::dependency<S3OptionsService>> {};

    struct AmqpOptionsService
        : kgr::single_service<
            Core::AMQP::Config::AmqpOptions,
            kgr::dependency<ConfigServiceService>> {};

    struct AmqpContextService
        : kgr::single_service<
            Core::AMQP::AmqpContext, 
            kgr::dependency<AmqpOptionsService>> {};

    struct AmqpConsumerService
        : kgr::single_service<
            Core::AMQP::AmqpConsumer, 
            kgr::dependency<AmqpContextService>> {};

    struct WebsocketServerService : kgr::single_service<Core::Websocket::WebsocketServer> {};
}