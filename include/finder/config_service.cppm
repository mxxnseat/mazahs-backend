module;

#include <optional>
export module config;

import std;

export class ConfigService{
    public:
        static std::optional<std::string> get(const std::string& key, std::optional<std::string> default_value = std::nullopt){
            const char* val = std::getenv(key.c_str());
            return val ? std::string(val) : default_value;
        }

        static std::string require(const std::string& key) {
            const char* val = std::getenv(key.c_str());
            if (!val) {
                throw std::runtime_error("Missing env: " + key);
            }
            return std::string(val);
        }
};