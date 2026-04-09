module;

#include <string>

export module core.kafka.config;

import config;


export namespace Core::Kafka::Config {
    struct KafkaProducerOptions {
        std::string bootstrap_servers;
        std::string acks;

        KafkaProducerOptions(const ConfigService& config_service)
            : bootstrap_servers(config_service.require("KAFKA_BOOTSTRAP_SERVERS")),
              acks("all") {}
    };
}