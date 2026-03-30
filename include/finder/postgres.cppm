module;

#include <pqxx/pqxx>

export module postgres;

import std;

export class Postgres {
    public:
        void connect(std::string connection_string);
        void close();
        void health_check();

        pqxx::connection& get_connection() const;
    private:
        std::unique_ptr<pqxx::connection> _connection;
};
