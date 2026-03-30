module;

#include <nlohmann/json.hpp>
#include <map>
#include <websocketpp/server.hpp>
#include <iostream>
#include <memory>
#include <vector>

export module websocket.songs;

import core.config;
import core.websocket_server;
import domain.songs;
import searcher.songs.dsp;

export namespace Websocket::Songs {
    enum class ClientBufferStatus {
        IDLE,
        PROCESSING
    };

    struct ClientBuffer {
        std::vector<float> data;
        int buffer_size;
        int samplerate;
        ClientBufferStatus status;
    };

    class SongsWebsocketMethod : public Core::Websocket::WebsocketMethod<std::vector<float>> {
        public:
            SongsWebsocketMethod(
                Searcher::Songs::DSP::ResampleNode& resample_node,
                Searcher::Songs::DSP::STFTNode& stft_node,
                Searcher::Songs::DSP::ExtractPeaksNode& extract_peaks_node,
                Searcher::Songs::DSP::BuildHashNode& build_hash_node,
                Searcher::Songs::DSP::PeakNormalizationNode& peak_normalization_node,
                Searcher::Songs::DSP::BandpassFilterNode& bandpass_filter_node,
                Domain::Songs::Services::SongHashService& song_hash_service,
                Domain::Songs::Services::SongService& song_service
            );

            nlohmann::json process(websocketpp::connection_hdl hdl, const std::vector<float>& data) override;
            nlohmann::json join(websocketpp::connection_hdl hdl, const nlohmann::json& payload) override {
                // TODO: More validation
                if(!payload.contains("samplerate") || payload["samplerate"].is_null()){
                    return {
                        {"jsonrpc", "2.0"},
                        {"code", "-32602"},
                        {"message", "Sample rate not defined"}
                    };
                }
                int samplerate = payload["samplerate"];
                // TODO: I'm TIRED FROM THIS PROJECT, BUT IT SHOULD BE IN CONFIG
                int buffer_size = samplerate * 7; // 7 seconds
                std::vector<float> buffer;
                buffer.reserve(buffer_size);
                ClientBuffer client_buffer{
                    .data = std::move(buffer),
                    .buffer_size = buffer_size,
                    .samplerate = samplerate,
                    .status = ClientBufferStatus::IDLE
                };
                buffers.emplace(hdl, std::move(client_buffer));
                Core::Websocket::WebsocketMethod<std::vector<float>>::join(hdl);
                return {
                    {"jsonrpc", "2.0"},
                    {"result", nullptr},
                    {"id", nullptr}
                };
            }
        private:
            std::map<
                websocketpp::connection_hdl,
                ClientBuffer,
                std::owner_less<websocketpp::connection_hdl>
            > buffers;

            Searcher::Songs::DSP::ResampleNode& resample_node;
            Searcher::Songs::DSP::STFTNode& stft_node;
            Searcher::Songs::DSP::ExtractPeaksNode& extract_peaks_node;
            Searcher::Songs::DSP::BuildHashNode& build_hash_node;
            Searcher::Songs::DSP::PeakNormalizationNode& peak_normalization_node;
            Searcher::Songs::DSP::BandpassFilterNode& bandpass_filter_node;
            Domain::Songs::Services::SongHashService& song_hash_service;
            Domain::Songs::Services::SongService& song_service;
    };
}