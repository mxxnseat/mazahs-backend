module;

#include <memory>
#include <thirdparty/cppq/cppq.hpp>
#include <nlohmann/json.hpp>
#include <kangaru/kangaru.hpp>

export module jobs;

import std;
import redis;

namespace Jobs {
    export class IQueue {
        public:
            virtual ~IQueue() = default;
            virtual std::string get_queue_name() const = 0;
    };

    export template<typename T>
    class Queue : public IQueue{
        public:
            explicit Queue(const RedisClient& redis, std::string queue_name)
            : redis(redis), queue_name(queue_name) {}

            void add(std::string task, T& payload) {
                nlohmann::json j = payload;
                cppq::Task task_def{task, j.dump(), 10};
                cppq::enqueue(redis.get(), task_def, queue_name);
            };

            std::string get_queue_name() const {
                return queue_name;
            };
        private:
            const RedisClient& redis;
            std::string queue_name;
    };

    class IHandler {
        public:
            virtual ~IHandler() = default;
            virtual void bootstrap() const;
        private:
    };

    export template<typename T>
    class Handler : public IHandler {
        public:
            Handler(std::string task_name)
            : task_name(task_name) 
            {};

            void bootstrap() const override {
                cppq::registerHandler(task_name, [&](cppq::Task task){
                    try{
                        nlohmann::json parsedPayload = nlohmann::json::parse(task.payload);
                        T payload = parsedPayload.get<T>();
                        process(payload);
                    }catch (const nlohmann::json::exception& e) {
                        std::cout << "[JSON ERROR] " << e.what() << std::endl;
                        throw;
                    } catch (const std::exception& e) {
                        std::cout << "[ERROR] " << e.what() << std::endl;
                        throw;
                    } catch (...) {
                        std::cout << "[UNKNOWN ERROR]" << std::endl;
                        throw;
                    }
                });
            }

            // Actually maybe we need pass some object with job id and other fields
            virtual void process(const T& payload) const = 0;
        private:
            std::string task_name;
    };

    export class Server {
        public:
            Server(const RedisClient& redis): redis(redis) {}

            void register_queue(const IQueue& queue) {
                queues.push_back(queue);
            };

            void run() {
                std::map<std::string, int> queues_options;
                for(const IQueue& queue : queues) {
                    queues_options.emplace(queue.get_queue_name(), 10);
                }
                cppq::runServer(redis.get_options(), queues_options, 1000);
            };

        private:
            std::vector<std::reference_wrapper<const IQueue>> queues;
            const RedisClient& redis;
    };
};
