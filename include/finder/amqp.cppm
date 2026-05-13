module;

#include <ev.h>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>
#include <exception>
#include <string>
#include <nlohmann/json.hpp>

export module core.amqp;

import core.amqp.config;

export namespace Core::AMQP {
    template <typename Payload>
    class AmqpConsumerHandler {
        public:
            virtual int process(const Payload&) = 0;
    };

    void safe_amqp_connection_close(::AMQP::TcpConnection& connection) {
        if(!connection.closed()){
            connection.close();
        }
    }

    class AmqpContext {
        public:
            AmqpContext(const Core::AMQP::Config::AmqpOptions& options)
                : loop(EV_DEFAULT),
                handler(loop),
                connection(nullptr)
            {
                
                connect(options);
            }

            ~AmqpContext() {
                safe_amqp_connection_close(*connection);
            }

            void start() const {
                ev_run(loop, 0);
            }

            void connect(const Core::AMQP::Config::AmqpOptions& options) {
                ::AMQP::Address address(
                    std::format("amqp://{}:{}@{}:{}/{}",
                        options.username.value(),
                        options.password.value(),
                        options.host,
                        options.port,
                        options.vhost
                    )
                );

                connection = std::make_unique<::AMQP::TcpConnection>(&handler, address);
            }

            ::AMQP::TcpConnection& get_connection() const {
                return *connection.get();
            }
        private:
            struct ev_loop* loop;
            ::AMQP::LibEvHandler handler;
            std::unique_ptr<::AMQP::TcpConnection> connection;
    };

    class AmqpConsumer {
        public:
            AmqpConsumer(const AmqpContext& amqp_context ):
                connection(amqp_context.get_connection()),
                channel(::AMQP::TcpChannel(&connection))
            {}

            ~AmqpConsumer() {
                channel.close();
                safe_amqp_connection_close(connection);
            }

            template <typename Payload>
            void subscribe(std::string queue, AmqpConsumerHandler<Payload>& handler) {
                channel.consume(queue)
                    .onSuccess([this](const std::string &tag) {
                        this->onSuccess(tag);
                    })
                    .onCancelled([this](const std::string &tag) {
                        this->onCancelled(tag);
                    })
                    .onReceived([this, &handler](const ::AMQP::Message &msg,
                               uint64_t deliveryTag,
                               bool redelivered)
                    {
                        try {
                            std::string body(msg.body(), msg.bodySize());
                            std::cout << "Received: " << body << std::endl;
                            
                            nlohmann::json parsedPayload = nlohmann::json::parse(body);
                            Payload payload = parsedPayload.get<Payload>();
                            handler.process(payload);

                            channel.ack(deliveryTag);
                        } catch(std::exception& ex) {
                            std::cerr << "Error: " << ex.what() << std::endl;
                            channel.reject(deliveryTag);
                        }
                    })
                    .onError([this](const char *message) {
                        this->onError(message);
                    });
            }

        private:
            ::AMQP::TcpConnection& connection;
            ::AMQP::TcpChannel channel;

            void onSuccess(const std::string &tag) const {
                std::cout << "consumer " << tag << " started" << std::endl;
            }

            void onCancelled(const std::string &tag) const {
                std::cout << "consumer " << tag << " cancelled" << std::endl;
            }

            void onError(const char *message) const {
                std::cerr << "error starting consumer: " << message << std::endl;
            }
    };
}