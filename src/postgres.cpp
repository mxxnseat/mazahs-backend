module;

#include <pqxx/pqxx>
#include <memory>

module postgres;

import std;


void Postgres::connect(std::string connection_string) {
    if (_connection != nullptr && _connection->is_open()) {
        return;
    }

    _connection = std::make_unique<pqxx::connection>(connection_string);
}

void Postgres::close() {
    _connection->close();
}

pqxx::connection& Postgres::get_connection() const {
    return *_connection;
}

void Postgres::health_check() {}