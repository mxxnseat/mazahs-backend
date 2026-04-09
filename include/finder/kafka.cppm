module;

#include <cstring>
#include <optional>
#include <stdexcept>
#include <iostream>
#include <string>
#include <librdkafka/rdkafka.h>

export module core.kafka;

import core.kafka.config;

namespace Core::Kafka {
    static void set_config(rd_kafka_conf_t *conf, const char *key, const char* value) {
        char errstr[512];
        rd_kafka_conf_res_t res;

        res = rd_kafka_conf_set(conf, key, value, errstr, sizeof(errstr));
        if (res != RD_KAFKA_CONF_OK) {
            std::cerr << "Unable to set config: " << errstr << std::endl;
            exit(1);
        }
    }

    static void dr_msg_cb (rd_kafka_t *kafka_handle,
                       const rd_kafka_message_t *rkmessage,
                       void *opaque) {
        if (rkmessage->err) {
            std::cerr << "Message delivery failed: " << rd_kafka_err2str(rkmessage->err) << std::endl;
        }
    }


    export class KafkaProducer {
        public:
            KafkaProducer(const Config::KafkaProducerOptions& options){
                char errstr[512];
                rd_kafka_conf_t *conf = rd_kafka_conf_new();

                set_config(conf, "bootstrap.servers", options.bootstrap_servers.c_str());
                set_config(conf, "acks",              options.acks.c_str());

                rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);

                producer = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));

                if (!producer) {
                    std::cerr << "Failed to create new producer: " << errstr << std::endl;
                    throw std::runtime_error(errstr);
                }

                conf = NULL;
            }

            int produce(std::string topic, std::optional<std::string> key, std::string value) {
                rd_kafka_resp_err_t err;
                if (key) {
                    err = rd_kafka_producev(
                        producer,
                        RD_KAFKA_V_TOPIC(topic.c_str()),
                        RD_KAFKA_V_KEY(key->c_str(), key->size()),
                        RD_KAFKA_V_VALUE(value.data(), value.size()),
                        RD_KAFKA_V_END
                    );
                } else {
                    err = rd_kafka_producev(
                        producer,
                        RD_KAFKA_V_TOPIC(topic.c_str()),
                        RD_KAFKA_V_VALUE(value.data(), value.size()),
                        RD_KAFKA_V_END
                    );
                }
                if (err) {
                    std::cerr << "Failed to produce to topic: " << topic << " " << rd_kafka_err2str(err) << std::endl;
                    return 1;
                } else {
                    std::cerr << "Produced event to topic: " << topic << " key: " << (key.has_value() ? key.value() : "nullptr ") << "value: " << rd_kafka_err2str(err) << std::endl;
                }

                rd_kafka_poll(producer, 0);
                rd_kafka_flush(producer, 10 * 1000);
                return 0;
            }
        private:
            rd_kafka_t *producer;
    };
}