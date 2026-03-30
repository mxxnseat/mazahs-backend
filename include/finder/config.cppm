module;

export module core.config;

import std;

export namespace Core::Config {
    std::string DATABASE_URL();

    std::string REDIS_HOST();
    int REDIS_PORT();

    std::string DOWNLOAD_DIRECTORY();

    int WORKING_SAMPLERATE();

    int STFT_WINDOW_LENGTH();
    int STFT_HOP_LENGTH();
}