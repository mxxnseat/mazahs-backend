#include <kangaru/kangaru.hpp>

import core.websocket_server;
import websocket.songs.ioc;
import websocket.songs;
import core.ioc;
import postgres;
import core.config;

int main() {
    kgr::container container;

    Postgres& postgres = container.service<Core::IoC::PostgresService>();
    postgres.connect(Core::Config::DATABASE_URL());
    auto& websocketServer = container.service<Core::IoC::WebsocketServerService>();
    auto& songsWebsocketMethod = container.service<Websocket::Songs::IoC::SongsWebsocketMethodService>();

    websocketServer.add_method("songs", songsWebsocketMethod);

    websocketServer.run();

    return 0;
}