#ifndef S3_GATEWAY_CONFIG_H_
#define S3_GATEWAY_CONFIG_H_

#include <cstdint>
#include <string>
#include <vector>

namespace s3_gateway {

constexpr const char* kDefaultManifestKey = ".gateway/files.json";

struct BucketConfig {
  std::string name;
  std::string manifest_key = kDefaultManifestKey;
  bool is_public = false;
};

struct Config {
  std::string aws_region = "us-east-1";
  std::string s3_endpoint;
  std::vector<BucketConfig> buckets;
  std::string host = "0.0.0.0";
  uint16_t port = 8080;

  bool HasCustomEndpoint() const { return !s3_endpoint.empty(); }
  const BucketConfig* FindBucket(const std::string& bucket_name) const;
  bool IsBucketPublic(const std::string& bucket_name) const;

  static Config FromEnv();
};

}  // namespace s3_gateway

#endif  // S3_GATEWAY_CONFIG_H_
