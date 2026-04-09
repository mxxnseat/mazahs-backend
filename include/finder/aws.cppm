module;

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <fstream>
#include <iostream>
#include <memory>


export module core.aws;

import core.aws.config;


export namespace Core::AWS {
    class S3Client {
        public:
            S3Client(const Config::S3Options& options) : options(options) {
                Aws::InitAPI(sdk_options);
                Aws::Client::ClientConfiguration config;

                config.region = options.region;

                client = std::make_unique<Aws::S3::S3Client>(config);
            }
            ~S3Client(){
                client.reset();
                Aws::ShutdownAPI(sdk_options);
            }

            int putObjectStream(std::string key, std::string file) {
                Aws::S3::Model::PutObjectRequest request;
                request.SetBucket(options.bucket);
                request.SetKey(key);

                auto body = Aws::MakeShared<Aws::FStream>(
                    "PutObjectInputStream",
                    file,
                    std::ios_base::in | std::ios_base::binary
                );

                request.SetBody(body);

                auto outcome = client->PutObject(request);
                if (!outcome.IsSuccess()) {
                    std::cerr << outcome.GetError().GetExceptionName()
                            << ": " << outcome.GetError().GetMessage() << "\n";
                    return 1;
                }
                return 0;
            }
        
        private:
            std::unique_ptr<Aws::S3::S3Client> client;
            Aws::SDKOptions sdk_options;
            const Config::S3Options& options;
    };
}