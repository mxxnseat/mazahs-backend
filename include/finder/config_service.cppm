module;

export module config;

import std;

export class ConfigService{
    public:
        static std::string get(const std::string& key, const std::string& default_value = "");

        static std::string require(const std::string& key);
};