module;

#include "aws/core/utils/memory/stl/AWSStringStream.h"
#include "aws/core/utils/memory/stl/AWSVector.h"
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CompletedPart.h>
#include <aws/s3/model/UploadPartRequest.h>
#include <aws/s3/model/UploadPartResult.h>
#include <aws/s3/model/CompleteMultipartUploadRequest.h>
#include <aws/s3/model/AbortMultipartUploadRequest.h>
#include <aws/s3/model/CreateMultipartUploadRequest.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <iostream>
#include <memory>
#include <vector>
#include <stdexcept>


export module core.aws;

import core.aws.config;


export namespace Core::AWS {
    class S3Client {
        public:
            S3Client(const Config::S3Options& options) : options(options) {
                Aws::InitAPI(sdk_options);
                Aws::S3::S3ClientConfiguration config;

                config.region = options.region;
                resolve_endpoint(options, config);

                client = std::make_unique<Aws::S3::S3Client>(config);
            }
            ~S3Client(){
                client.reset();
                Aws::ShutdownAPI(sdk_options);
            }

            Aws::String create_multipart_upload(const Aws::String& bucket, const Aws::String& key) const {
                Aws::S3::Model::CreateMultipartUploadRequest request;
                request.SetBucket(bucket);
                request.SetKey(key);

                auto outcome = client->CreateMultipartUpload(request);
                if (!outcome.IsSuccess()) {
                    std::cerr << "Failed to create multipart upload: " << outcome.GetError().GetMessage() << std::endl;
                    throw std::runtime_error("Failed to create multipart upload");
                }

                return outcome.GetResult().GetUploadId();
            }
            
            Aws::S3::Model::UploadPartResult upload_part(
                const Aws::String& bucket,
                const Aws::String& key, 
                const Aws::String& upload_id,
                int part_number, 
                const char* data, 
                int chunk_size
            ) const {
                Aws::S3::Model::UploadPartRequest request;
                request.SetBucket(bucket.c_str());
                request.SetKey(key.c_str());
                request.SetPartNumber(part_number);
                request.SetUploadId(upload_id.c_str());

                std::shared_ptr<Aws::StringStream> stream =
                Aws::MakeShared<Aws::StringStream>("UploadStream");

                stream->write(data,static_cast<std::streamsize>(chunk_size));
                stream->seekg(0, std::ios::beg);
                request.SetBody(stream);

                request.SetContentLength(chunk_size);

                auto result = client->UploadPart(request);
                if(!result.IsSuccess()){
                    std::cerr << "Failed to upload part " << part_number << ": " << result.GetError().GetMessage() << std::endl;
                    throw std::runtime_error("Failed to upload part");
                }
                return result.GetResult();
            }

            void complete_multipart_upload(const Aws::String& bucket, const Aws::String& key, const Aws::String& upload_id, const Aws::Vector<Aws::S3::Model::CompletedPart>& completed_parts) const {
                Aws::S3::Model::CompleteMultipartUploadRequest completeMultipartUploadRequest;
                completeMultipartUploadRequest.SetBucket(bucket.c_str());
                completeMultipartUploadRequest.SetKey(key.c_str());
                completeMultipartUploadRequest.SetUploadId(upload_id.c_str());

                Aws::S3::Model::CompletedMultipartUpload completedMultipartUpload;
                completedMultipartUpload.SetParts(completed_parts);

                completeMultipartUploadRequest.WithMultipartUpload(completedMultipartUpload);

                auto completeMultipartUploadOutcome =
                client->CompleteMultipartUpload(completeMultipartUploadRequest);
            }

            void abort_multipart_upload(const Aws::String& bucket, const Aws::String& key, const Aws::String& upload_id) const {
                Aws::S3::Model::AbortMultipartUploadRequest abortMultipartUploadRequest;
                abortMultipartUploadRequest.SetBucket(bucket.c_str());
                abortMultipartUploadRequest.SetKey(key.c_str());
                abortMultipartUploadRequest.SetUploadId(upload_id.c_str());

                auto abortMultipartUploadOutcome =
                client->AbortMultipartUpload(abortMultipartUploadRequest);
            }
        
        private:
            void resolve_endpoint(const Config::S3Options& config, Aws::S3::S3ClientConfiguration& output) {
                if(config.endpoint.has_value()){
                    output.endpointOverride = config.endpoint.value();
                    output.scheme = Aws::Http::Scheme::HTTP;
                    output.useVirtualAddressing = false;
                }
            }

            std::unique_ptr<Aws::S3::S3Client> client;
            Aws::SDKOptions sdk_options;
            const Config::S3Options& options;
    };

    class S3MultipartUploader {
        public:
            S3MultipartUploader(const S3Client& s3_client, const Aws::String& bucket, const Aws::String& key, int chunk_size = 5 * 1024 * 1024)
                : s3_client(s3_client), bucket(bucket), key(key), chunk_size(chunk_size) { 
                    buffer.reserve(chunk_size);

                    begin();
                }

            ~S3MultipartUploader(){
                try{
                    if(!buffer.empty()){
                        upload_part();
                    }
                    finish();
                }catch(const std::exception& e){
                    std::cerr << "Failed to complete multipart upload: " << e.what() << std::endl;
                    s3_client.abort_multipart_upload(bucket, key, upload_id);
                }
            }

            void write(const char* data, std::streamsize n) {
                buffer.insert(buffer.end(), data, data + n);

                while(buffer.size() >= chunk_size){
                    upload_part();
                }
            }
        private:
            void upload_part() {
                auto upload_part_result = s3_client.upload_part(
                    bucket,
                    key, 
                    upload_id,
                    ++part_number,
                    buffer.data(), 
                    buffer.size() < chunk_size ? buffer.size() : chunk_size);

                Aws::S3::Model::CompletedPart completed;
                completed.SetPartNumber(part_number);
                completed.SetETag(upload_part_result.GetETag());
                completed_parts.push_back(completed);
                flush_part();
            }
            void begin() {
                try{
                    upload_id = s3_client.create_multipart_upload(bucket, key);
                }catch(const std::exception& e){
                    std::cerr << "Multipart upload failed: " << e.what() << std::endl;
                    throw std::runtime_error("Failed to begin multipart upload");
                }
            }
            void finish() {
                try{
                    s3_client.complete_multipart_upload(bucket, key, upload_id, completed_parts);
                }catch(const std::exception& e){
                    throw std::runtime_error("Failed to complete multipart upload");
                }
            } 
            void flush_part() {
                if(buffer.size() > chunk_size){
                    buffer.erase(buffer.begin(), buffer.begin() + chunk_size);
                }else{
                    buffer.clear();
                }
            }

            int chunk_size;
            std::vector<char> buffer;

            Aws::Vector<Aws::S3::Model::CompletedPart> completed_parts;
            const S3Client& s3_client;
            Aws::String bucket;
            Aws::String key;

            Aws::String upload_id;
            int part_number = 0;
    };
}