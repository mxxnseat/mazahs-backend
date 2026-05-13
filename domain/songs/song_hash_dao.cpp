module;

#include <optional>
#include <pqxx/pqxx>
#include <vector>
#include <unordered_map>

module domain.songs;

import searcher.songs;

using namespace Domain::Songs;


DAOs::SongHashDAO::SongHashDAO(const Postgres& postgres) : postgres(postgres), table("song_hashes") {}

// TODO: Execute it in the passed transaction
std::vector<Entities::SongHash::DTO> DAOs::SongHashDAO::create_many(const std::vector<Entities::SongHash::CreatePayload>& payload) {
    try{
        pqxx::work trx(postgres.get_connection());
        pqxx::params params;
        pqxx::placeholders placeholders;

        std::unordered_map<std::string, int> first_fields =
            Entities::SongHash::CreatePayload::to_fields(payload[0]);

        std::vector<std::string> columns;
        columns.reserve(first_fields.size());

        for (const auto& [column, _] : first_fields) {
            columns.push_back(column);
        }

        std::string columns_str;
        for (int i = 0; i < columns.size(); ++i) {
            if (i > 0) {
                columns_str += ",";
            }
            columns_str += columns[i];
        }

        std::string values_str;

        for (int row_index = 0; row_index < payload.size(); ++row_index) {
            auto fields = Entities::SongHash::CreatePayload::to_fields(payload[row_index]);

            if (fields.size() != columns.size()) {
                throw std::runtime_error("insert_many: inconsistent field count");
            }

            if (row_index > 0) {
                values_str += ",";
            }

            values_str += "(";

            for (int col_index = 0; col_index < columns.size(); ++col_index) {
                const std::string& column = columns[col_index];

                auto it = fields.find(column);
                if (it == fields.end()) {
                    throw std::runtime_error("insert_many: missing field '" + column + "'");
                }

                params.append(it->second);

                if (col_index > 0) {
                    values_str += ",";
                }

                values_str += placeholders.get();
                placeholders.next();
            }

            values_str += ")";
        }

        std::string query = std::format(R"(
            insert into {} ({})
            values {}
            returning *;
        )", table, columns_str, values_str);

        pqxx::result result = trx.exec(query, params);
        trx.commit();

        std::vector<Entities::SongHash::DTO> out;
        out.reserve(result.size());

        for (const auto& row : result) {
            out.push_back(Entities::SongHash::DTO::from_row(row));
        }

        return out;
    }catch(pqxx::failure& ex) {
        std::cout << ex.what() << std::endl;
        throw;
    }catch(...){
        std::cout << "Unknown error" << std::endl;
        throw;
    }
}


std::optional<Entities::SongHash::VoteResult> DAOs::SongHashDAO::vote(const std::vector<Searcher::Songs::DSP::BuildHashOutput>& payload){
    try {
        if(payload.empty()){
            return std::nullopt;
        }
        pqxx::work trx(postgres.get_connection());
        pqxx::params params;
        pqxx::placeholders placeholders;

        std::string values_str;
        // values_str.reserve(payload.size() * 50);

        for (size_t i = 0; i < payload.size(); ++i) {
            if (i > 0) {
                values_str += ",";
            }

            values_str += "(";
            values_str += std::format("{}::int", placeholders.get());
            placeholders.next();
            values_str += ",";
            values_str += std::format("{}::bigint", placeholders.get());
            placeholders.next();
            values_str += ")";

            params.append(payload[i].hash);
            params.append(payload[i].time);
        }
        std::string query = std::format(R"(
            with query_hashes(hash, query_time) as (
                values {}
            ),
            matches as (
                select
                    sh.song_id,
                    sh.anchor_time - qh.query_time as time_offset,
                    sh.anchor_time as anchor_time
                from query_hashes qh
                join song_hashes sh
                    on sh.hash = qh.hash
            ),
            offset_votes as (
                select
                    song_id,
                    time_offset,
                    avg(anchor_time)::int as anchor_time,
                    count(*) as votes
                from matches
                group by song_id, time_offset
            ),
            best_per_song as (
                select distinct on (song_id)
                    song_id,
                    time_offset,
                    votes,
                    anchor_time
                from offset_votes
                where votes > 20
                order by song_id, votes desc
            )
            select
                s.id,
                s.url,
                s.status,
                b.votes,
                b.anchor_time
            from best_per_song b
            join songs s
                on s.id = b.song_id
            order by b.votes desc
            limit 1;
        )", values_str);

        // std::cout << query << std::endl;
        pqxx::result result = trx.exec(query, params);
        trx.commit();
        if(result.empty()){
            return std::nullopt;
        }
        return Entities::SongHash::VoteResult::from_row(result[0]);
    }catch(pqxx::failure& ex) {
        std::cout << ex.what() << std::endl;
        throw;
    }catch(...){
        std::cout << "Unknown error" << std::endl;
        throw;
    }
}