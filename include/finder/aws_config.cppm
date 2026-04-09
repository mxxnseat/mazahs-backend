module;

#include <optional>
#include <string>

export module core.aws.config;

import config;


export namespace Core::AWS::Config {
    struct S3Options {
        std::string region;
        std::string bucket;
        std::optional<std::string> endpoint;

        S3Options(const ConfigService& config_service) : 
        region(config_service.get("AWS_REGION", "eu-west-2").value()),
        bucket(config_service.require("AWS_BUCKET")),
        endpoint(config_service.get("AWS_ENDPOINT"))
        {}
    };
}