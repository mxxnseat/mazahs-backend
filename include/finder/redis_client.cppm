module;

#include <hiredis/hiredis.h>
#include <kangaru/kangaru.hpp>

export module redis;

import std;

export class RedisClient {
    public:
        RedisClient();

        ~RedisClient();

        redisContext* get() const;
        const redisOptions& get_options() const;
    private:
        redisContext* context;
        redisOptions options;
        std::string host;
        int port{};
};