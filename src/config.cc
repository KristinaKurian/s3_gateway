#include "config.h"

#include <cstdlib>
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

}  // namespace

Config Config::FromEnv() {
  Config config;
  config.aws_region = GetEnvOr("AWS_REGION", "us-east-1");
  config.s3_endpoint = GetEnvOr("S3_ENDPOINT", "");
  config.host = GetEnvOr("HOST", "0.0.0.0");

  std::string port_str = GetEnvOr("PORT", "8080");
  int port = std::stoi(port_str);
  if (port <= 0 || port > 65535) {
    throw std::runtime_error("PORT must be in range 1..65535");
  }
  config.port = static_cast<uint16_t>(std::stoi(port_str));

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
      bucket.name = parts[0];
      if (parts.size() > 1) {
        bucket.manifest_key = parts[1];
      } else {
        bucket.manifest_key = ".parparchik/files.json";
      }
      if (parts.size() > 2 && parts[2] == "public") {
        bucket.is_public = true;
      } else {
        bucket.is_public = false;
      }
      config.buckets.push_back(bucket);
    }
  } else {
    // Backward compatibility
    std::string default_manifest = ".parparchik/files.json";
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
