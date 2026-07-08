#include "config.h"

#include <algorithm>
#include <unordered_set>
#include <cstdlib>
#include <charconv>
#include <sstream>
#include <stdexcept>

namespace s3_gateway {

namespace {

std::string GetEnvOr(const char* name, const std::string& default_value) {
  const char* value = std::getenv(name);
  if (value == nullptr || value[0] == '\0') {
    return default_value;
  }
  return value;
}

std::string Trim(const std::string& value) {
  const auto begin = value.find_first_not_of(" \t\n\r");
  if (begin == std::string::npos) {
    return "";
  }

  const auto end = value.find_last_not_of(" \t\n\r");
  return value.substr(begin, end - begin + 1);
}

uint16_t ParsePort(const std::string& value) {
  int port = 0;

  const auto* begin = value.data();
  const auto* end = value.data() + value.size();

  auto [ptr, ec] = std::from_chars(begin, end, port);

  if (ec != std::errc{} || ptr != end || port <= 0 || port > 65535) {
    throw std::runtime_error("PORT must be an integer in range 1..65535");
  }

  return static_cast<uint16_t>(port);
}

}  // namespace

const BucketConfig* Config::FindBucket(const std::string& bucket_name) const {
  const auto it = std::find_if(
      buckets.begin(),
      buckets.end(),
      [&bucket_name](const BucketConfig& bucket) {
        return bucket.name == bucket_name;
      });

  if (it == buckets.end()) {
    return nullptr;
  }

  return &(*it);
}

bool Config::IsBucketPublic(const std::string& bucket_name) const {
    const BucketConfig* bucket = FindBucket(bucket_name);

    if (bucket == nullptr) {
      return false;
    }

    return bucket->is_public;
}

Config Config::FromEnv() {
  Config config;
  config.aws_region = GetEnvOr("AWS_REGION", "us-east-1");
  config.s3_endpoint = GetEnvOr("S3_ENDPOINT", "");
  config.host = GetEnvOr("HOST", "0.0.0.0");

  int port = ParsePort(GetEnvOr("PORT", "8080"));
  config.port = static_cast<uint16_t>(port);

  std::string buckets_str = GetEnvOr("BUCKETS", "");
  if (!buckets_str.empty()) {
    std::istringstream ss(buckets_str);
    std::string token;
    while (std::getline(ss, token, ',')) {
      BucketConfig bucket;
      std::istringstream token_ss(token);
      std::string part;
      std::vector<std::string> parts;
      while (std::getline(token_ss, part, ':')) {
        parts.push_back(part);
      }
      if (parts.empty()) continue;
      bucket.name = Trim(parts[0]);
      if (bucket.name.empty()) {
        throw std::runtime_error("Bucket name in BUCKETS must not be empty");
      }
      if (parts.size() > 1) {
        const auto manifest_key = Trim(parts[1]);
        if (!manifest_key.empty()) {
          bucket.manifest_key = manifest_key;
        }
      }
      if (parts.size() > 2 && parts[2] == "public") {
        bucket.is_public = true;
      }
      config.buckets.push_back(bucket);
    }
  } else {
    // Backward compatibility
    std::string default_manifest = kDefaultManifestKey;
    const char* pub = std::getenv("PUBLIC_BUCKET");
    const char* priv = std::getenv("PRIVATE_BUCKET");
    if (!pub || pub[0] == '\0' || !priv || priv[0] == '\0') {
      throw std::runtime_error(
          "Set BUCKETS or both PUBLIC_BUCKET and "
          "PRIVATE_BUCKET");
    }
    config.buckets.push_back({pub, default_manifest, true});
    config.buckets.push_back({priv, default_manifest, false});
  }

  if (config.buckets.empty()) {
    throw std::runtime_error("At least one bucket must be configured");
  }

  return config;
}

}  // namespace s3_gateway
