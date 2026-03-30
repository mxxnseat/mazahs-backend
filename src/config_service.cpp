module;

module config;

import std;

std::string ConfigService::get(const std::string& key, const std::string& default_value){
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : default_value;
}

std::string ConfigService::require(const std::string& key) {
    const char* val = std::getenv(key.c_str());
    if (!val) {
        throw std::runtime_error("Missing env: " + key);
    }
    return std::string(val);
}