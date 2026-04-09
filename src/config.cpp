module;

module core.config;

import config;
import std;

std::string Core::Config::DATABASE_URL() {
    return ConfigService::require("DATABASE_URL");
}

std::string Core::Config::REDIS_HOST() {
    return ConfigService::require("REDIS_HOST");
}

int Core::Config::REDIS_PORT() {
    return std::stoi(ConfigService::require("REDIS_PORT"));
}

std::string Core::Config::DOWNLOAD_DIRECTORY(){
    return ConfigService::get("DOWNLOAD_DIRECTORY", "/tmp").value();
}

int Core::Config::WORKING_SAMPLERATE(){
        return std::stoi(ConfigService::get("WORKING_SAMPLERATE", "22050").value());
}

int Core::Config::STFT_WINDOW_LENGTH() {
    return std::stoi(ConfigService::get("STFT_WINDOW_LENGTH", "2048").value());
}

int Core::Config::STFT_HOP_LENGTH() {
    return std::stoi(ConfigService::get("STFT_HOP_LENGTH", "512").value());
}
