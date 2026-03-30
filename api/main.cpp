#include <thirdparty/cpp-httplib/httplib.h>
#include <nlohmann/json.hpp>
#include <kangaru/kangaru.hpp>

using namespace httplib;

import core.ioc;
import core.config;

import jobs;
import searcher.songs;
import searcher.songs.ioc;
import redis;
import std;


// JUST KEEP IT SIMPLE

int main(void){
    kgr::container container;
    Server api_server;

    auto& queue = container.service<Searcher::Songs::IoC::SongsQueueService>();
    if (!api_server.is_valid()) {
        std::cout << "server has an error..." << std::endl;
        return -1;
    }

    api_server.set_pre_routing_handler([&](const Request& request,Response& response){
        response.set_header("Access-Control-Allow-Origin", "*");
        response.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        response.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        if (request.method == "OPTIONS") {
            response.status = 204;
            return Server::HandlerResponse::Handled;
        }
        return Server::HandlerResponse::Unhandled;
    });

    api_server.Put("/api/songs", [&](const Request& request, Response& response){
        nlohmann::json j = nlohmann::json::parse(request.body);

        Searcher::Songs::SongsPullPayload payload = {
            .url = j["url"]
        };
        
        queue.add(Searcher::Songs::Constants::SONGS_PULL_TASK_NAME, payload);

        response.status = 202;
    });

    if (!api_server.listen("0.0.0.0", 9000)) {
        std::cerr << "failed to start server" << std::endl;
        return -1;
    }
    return 0;
}