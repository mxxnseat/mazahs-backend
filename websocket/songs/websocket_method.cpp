module;

#include <iostream>
#include <nlohmann/json.hpp>
#include <websocketpp/server.hpp>
#include <Eigen/dense>
#include <vector>

module websocket.songs;

import core.websocket_server;
import searcher.songs.dsp;
import domain.songs;


Websocket::Songs::SongsWebsocketMethod::SongsWebsocketMethod(
        Searcher::Songs::DSP::ResampleNode& resample_node,
        Searcher::Songs::DSP::STFTNode& stft_node,
        Searcher::Songs::DSP::ExtractPeaksNode& extract_peaks_node,
        Searcher::Songs::DSP::BuildHashNode& build_hash_node,
        Searcher::Songs::DSP::PeakNormalizationNode& peak_normalization_node,
        Searcher::Songs::DSP::BandpassFilterNode& bandpass_filter_node,
        Domain::Songs::Services::SongHashService& song_hash_service,
        Domain::Songs::Services::SongService& song_service
    ) :
    Core::Websocket::WebsocketMethod<std::vector<float>>(),
    resample_node(resample_node),
    stft_node(stft_node),
    extract_peaks_node(extract_peaks_node),
    build_hash_node(build_hash_node),
    peak_normalization_node(peak_normalization_node),
    bandpass_filter_node(bandpass_filter_node),
    song_hash_service(song_hash_service),
    song_service(song_service) {}


nlohmann::json Websocket::Songs::SongsWebsocketMethod::process(websocketpp::connection_hdl hdl, const std::vector<float>& data) {
    Websocket::Songs::ClientBuffer& client_buffer = buffers[hdl];
    if(client_buffer.status == Websocket::Songs::ClientBufferStatus::PROCESSING){
         return {
            {"jsonrpc", "2.0"},
            {"result", {
                "status", "processing"
            }},
            {"id", nullptr}
        };
    }
    if(client_buffer.data.size() < client_buffer.buffer_size) {
        client_buffer.data.insert(client_buffer.data.end(), data.begin(), data.end());
        return {
            {"jsonrpc", "2.0"},
            {"result", {
                {"status", "listening"}
            }},
            {"id", nullptr}
        };
    }

    client_buffer.status = Websocket::Songs::ClientBufferStatus::PROCESSING;

    std::vector<float> resampled_data = resample_node.process({
        .data = client_buffer.data, 
        .samplerate = client_buffer.samplerate
    });
    // std::vector<float> filtered_data = bandpass_filter_node.process(resampled_data);
    std::vector<float> normalized_data = peak_normalization_node.process(resampled_data);
    Eigen::MatrixXcf spectrogram = stft_node.process(normalized_data);
    std::vector<Searcher::Songs::DSP::ExtractPeaksOutput> peaks = extract_peaks_node.process(spectrogram);
    std::vector<Searcher::Songs::DSP::BuildHashOutput> hashes = build_hash_node.process(peaks);
    std::optional<Domain::Songs::Entities::SongHash::VoteResult> result = song_hash_service.vote(hashes);

    // AGAIN I'M SO TIRED FROM THIS PROJECT TO DO IT OKAY
    if(result.has_value()){
        return {
            {"jsonrpc", "2.0"},
            {"result", {
                {"status", "processed"},
                {"url", result->url},
                {"time", result->anchor_time * 512 / Core::Config::WORKING_SAMPLERATE() },
            }},
            {"id", nullptr}
        };
    }
    return {
        {"jsonrpc", "2.0"},
        {"result", {
            {"status", "processed"},
            {"url", nullptr}
        }},
        {"id", nullptr}
    };
}