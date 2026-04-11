module;

#include <optional>
#include <string>

export module core.aws.config;

import config;


export namespace Core::AWS::Config {
    struct S3Options {
        std::string region;
        std::string songs_bucket;
        std::optional<std::string> endpoint;

        S3Options(const ConfigService& config_service) : 
        region(config_service.get("AWS_REGION", "eu-west-2").value()),
        songs_bucket(config_service.get("AWS_SONGS_BUCKET", "songs").value()),
        endpoint(config_service.get("AWS_ENDPOINT"))
        {}
    };
}