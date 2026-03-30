module;

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <memory>
#include <set>
#include <map>
#include <nlohmann/json.hpp>

export module core.websocket_server;

import std;
import core.utils;

using json = nlohmann::json;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;


export namespace Core::Websocket {

    typedef websocketpp::server<websocketpp::config::asio> Server;

    typedef Server::message_ptr message_ptr;

    class IWebsocketMethod {
        public:
            virtual ~IWebsocketMethod() = default;
            virtual json call(websocketpp::connection_hdl hdl, const json& payload) = 0;
            virtual json call(websocketpp::connection_hdl hdl, const std::vector<float>& payload) = 0;
            virtual json join(websocketpp::connection_hdl hdl, const json& payload) = 0;
            virtual json join(websocketpp::connection_hdl hdl) = 0;
            virtual json leave(websocketpp::connection_hdl hdl) = 0;
    };

    template <typename T>
    class WebsocketMethod : public IWebsocketMethod {
        public:
            virtual json process(websocketpp::connection_hdl hdl, const T& data) = 0;
            json call(websocketpp::connection_hdl hdl, const json& payload) override {
                if(is_client_joined(hdl)) {
                    // Parse payload
                    return process(hdl, payload);
                }
                return {
                    {"jsonrpc", "2.0"},
                    {"code", "-32601"},
                    {"message", "Client not joined"},
                };
            }

            json call(websocketpp::connection_hdl hdl, const std::vector<float>& payload) override {
                 if(is_client_joined(hdl)) {
                    // Parse payload
                    return process(hdl, payload);
                }
                return {
                    {"jsonrpc", "2.0"},
                    {"code", "-32601"},
                    {"message", "Client not joined"},
                };
            }

            json join(websocketpp::connection_hdl hdl) override {
                clients.insert(hdl);
                return {
                    {"jsonrpc", "2.0"},
                    {"result", nullptr},
                    {"id", nullptr}
                };
            }
            
            json leave(websocketpp::connection_hdl hdl) override {
                clients.erase(hdl);
                 return {
                    {"jsonrpc", "2.0"},
                    {"result", nullptr},
                    {"id", nullptr}
                };
            }
        
        private:
            std::set<
                websocketpp::connection_hdl,
                std::owner_less<websocketpp::connection_hdl>
            > clients;

            bool is_client_joined(websocketpp::connection_hdl hdl) {
                return clients.find(hdl) != clients.end();
            }
    };


    class WebsocketServer {
        public:
            void run() {
                try {
                    // Set logging settings
                    // echo_server.set_access_channels(websocketpp::log::alevel::all);
                    // echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

                    // Initialize Asio
                    server.init_asio();

                    // Register our message handler
                    server.set_message_handler(bind(&WebsocketServer::on_message, this, ::_1, ::_2));
                    server.set_open_handler(bind(&WebsocketServer::on_open, this, ::_1));

                    // Listen on port 9002
                    server.listen(9002);

                    // Start the server accept loop
                    server.start_accept();

                    // Start the ASIO io_service run loop
                    server.run();
                } catch (websocketpp::exception const & e) {
                    std::cout << e.what() << std::endl;
                } catch (const std::exception& e) {
                    std::cout << "Error: " << e.what() << std::endl;
                }
            };
            void add_method(const std::string& method, IWebsocketMethod& websocket_method) {
                methods.insert({method, websocket_method});
            }
            void on_open(websocketpp::connection_hdl hdl) {
                // Add logging???
                clients.insert(hdl);
            }
            void on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
                try {
                    auto opcode = msg->get_opcode();
                    json result;
                    if(opcode == websocketpp::frame::opcode::binary){
                        auto room_iter = methods.find("songs");
                        if(room_iter == methods.end()){
                                // TODO: Handle this case;
                            return;
                        }
                        auto payload = msg->get_payload();
                        size_t count = payload.size() / sizeof(float);
                        std::vector<float> samples_f(count);
                        std::memcpy(samples_f.data(), payload.data(), payload.size());
                        result = room_iter->second.get().call(hdl, samples_f);
                    }
                    if(opcode == websocketpp::frame::opcode::text){
                        json payload = json::parse(msg->get_payload());
                        std::string method = payload["method"];
                        auto method_parts = Core::Utils::split(method, '.');

                        if(method_parts[method_parts.size()-1] == "join") {
                            auto room_iter = methods.find(method_parts[0]);
                            if(room_iter == methods.end()){
                                // TODO: Handle this case;
                                return;
                            }
                            if(payload.contains("params")) {
                                result = room_iter->second.get().join(hdl, payload["params"]);
                            } else{
                                result = room_iter->second.get().join(hdl);
                            }

                        } else if(method_parts[method_parts.size()-1] == "leave") {
                            auto room_iter = methods.find(method_parts[0]);
                            if(room_iter == methods.end()){
                                // TODO: Handle this case;
                                return;
                            }
                            result = room_iter->second.get().leave(hdl);
                        }else if(method_parts[method_parts.size()-1] == "call" && payload.contains("params")) {
                            auto room_iter = methods.find(method_parts[0]);
                            if(room_iter == methods.end()){
                                // TODO: Handle this case;
                                return;
                            }
                            room_iter->second.get().call(hdl, payload["params"]);
                        }
                    }
                    if(!result.is_null()){
                        server.send(hdl, result.dump(), websocketpp::frame::opcode::text);
                    }
                } catch (websocketpp::exception const & e) {
                    std::cout << "Echo failed because: "
                            << "(" << e.what() << ")" << std::endl;
                }
            }
        private:
            Server server;
            std::set<
                websocketpp::connection_hdl,
                std::owner_less<websocketpp::connection_hdl>
            > clients;
            std::map<
                std::string, 
                std::reference_wrapper<IWebsocketMethod>
            > methods;
    };
}
