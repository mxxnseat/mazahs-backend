module;

#include <optional>
#include <string>

export module core.amqp.config;

import config;


export namespace Core::AMQP::Config {
    struct AmqpOptions {
        std::optional<std::string> username;
        std::optional<std::string> password;
        std::string host;
        std::string vhost;
        std::string port;

        AmqpOptions(const ConfigService& config_service) : 
            username(config_service.get("AMQP_USERNAME")),
            password(config_service.get("AMQP_PASSWORD")),
            host(config_service.get("AMQP_HOST", "localhost").value()),
            vhost(config_service.get("AMQP_VHOST", "shazam").value()),
            port(config_service.get("AMQP_PORT", "5672").value())
        {}
    };
}