module;

#include <hiredis/hiredis.h>

module redis;

import core.config;
import std;

RedisClient::RedisClient() {
    host = Core::Config::REDIS_HOST();
    port = Core::Config::REDIS_PORT();

    options = redisOptions{};
    REDIS_OPTIONS_SET_TCP(&options, host.c_str(), port);

    context = redisConnectWithOptions(&options);

    if (!context || context->err) {
        throw std::runtime_error("Redis connection failed");
    }
}

RedisClient::~RedisClient() {
    if (context) {
        redisFree(context);
    }
}

redisContext* RedisClient::get() const{
    return context;
}

const redisOptions& RedisClient::get_options() const{
    return options;
}