module;

#include <kangaru/kangaru.hpp>

export module core.ioc;

import redis;
import postgres;
import jobs;
import core.websocket_server;

export namespace Core::IoC {
    namespace Utils {
        template<auto m>
        using method = kgr::method<decltype(m), m>;
    }

    struct PostgresService : kgr::single_service<Postgres> {};
    struct RedisClientService : kgr::single_service<RedisClient>{};
    struct JobsServerService : kgr::single_service<Jobs::Server, kgr::dependency<RedisClientService>> {};

    struct WebsocketServerService : kgr::single_service<Core::Websocket::WebsocketServer> {};
}