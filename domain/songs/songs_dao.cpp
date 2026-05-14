module;

#include <pqxx/pqxx>
#include <optional>
#include <format>

module domain.songs;

import std;


using namespace Domain::Songs;

DAOs::SongDAO::SongDAO(const Postgres& postgres) : postgres(postgres), table("songs") {}

std::optional<Entities::Song::DTO> DAOs::SongDAO::retrieve(int id) {
    try{
        pqxx::params params;
        pqxx::placeholders placeholders;
        pqxx::work trx(postgres.get_connection());
        std::string query = std::format(R"(
            select * from {} where id = {};
        )", table, id);
        pqxx::result result = trx.exec(query, params);
        trx.commit();
        if(result.empty()){
            return std::nullopt;
        }
        return Entities::Song::DTO::from_row(result[0]);
    }catch(pqxx::failure& ex) {
        std::cout << ex.what() << std::endl;
        throw;
    }catch(...){
        std::cout << "Unknown error" << std::endl;
        throw;
    }
}

Entities::Song::DTO DAOs::SongDAO::create(const Entities::Song::CreatePayload& payload) {
    try{
        pqxx::params params;
        pqxx::placeholders placeholders;
        pqxx::work trx(postgres.get_connection());
        std::unordered_map<std::string, std::string> fields = Entities::Song::CreatePayload::to_fields(payload);
        std::string columns_str = "";
        std::string placeholders_str = "";
        int ind = 0;
        for(const auto& field : fields){
            std::string column = field.first;
            std::string value = field.second;
            params.append(value);
            if(ind>0) {
                columns_str += ",";
                placeholders_str += ",";
            }
            columns_str += column;
            placeholders_str += placeholders.get();
            placeholders.next();
            ind++;
        }
        std::string query = std::format(R"(
            insert into {} ({})
            values ({})
            returning *;
        )", table, columns_str, placeholders_str);
        pqxx::result result = trx.exec(query, params);
        trx.commit();
        return Entities::Song::DTO::from_row(result[0]);
    }catch(pqxx::failure& ex) {
        std::cout << ex.what() << std::endl;
        throw;
    }catch(...){
        std::cout << "Unknown error" << std::endl;
        throw;
    }
}

Entities::Song::DTO DAOs::SongDAO::update(int id, const Entities::Song::UpdatePayload& payload) {
    try{
        pqxx::params params;
        pqxx::placeholders placeholders;
        pqxx::work trx(postgres.get_connection());
        std::unordered_map<std::string, std::string> fields = Entities::Song::UpdatePayload::to_fields(payload);
        std::string update_str;
        int ind = 0;
        for(const auto& field : fields){
            std::string column = field.first;
            std::string value = field.second;
            params.append(value);
            if(ind>0) {
                update_str += ",";
            }
            update_str += std::format("{} = {}", column, placeholders.get());
            placeholders.next();
            ind++;
        }
        params.append(id);
        std::string query = std::format(R"(
            update {} set {} where id = {} returning *;
        )", table, update_str, placeholders.get());
        pqxx::result result = trx.exec(query, params);
        trx.commit();
        return Entities::Song::DTO::from_row(result[0]);
    }catch(pqxx::failure& ex) {
        std::cout << ex.what() << std::endl;
        throw;
    }catch(...){
        std::cout << "Unknown error" << std::endl;
        throw;
    }
}
