#ifndef S3_GATEWAY_S3_CLIENT_H_
#define S3_GATEWAY_S3_CLIENT_H_

#include <optional>
#include <string>
#include <vector>

#include <aws/s3/S3Client.h>

namespace s3_gateway {

struct S3Object {
  std::string key;
  int64_t size;
  std::string last_modified;
};

class S3Client {
 public:
  S3Client(const std::string& region, const std::string& endpoint = "");

  std::vector<S3Object> ListObjects(const std::string& bucket) const;

  bool ObjectExists(const std::string& bucket,
                    const std::string& key) const;

  std::optional<S3Object> HeadObject(const std::string& bucket,
                                     const std::string& key) const;

  std::string GetObjectContent(const std::string& bucket,
                               const std::string& key) const;

  bool TryGetObjectContent(const std::string& bucket,
                           const std::string& key,
                           std::string* content) const;

  bool PutObjectContent(const std::string& bucket,
                        const std::string& key,
                        const std::string& content,
                        const std::string& content_type =
                            "application/octet-stream") const;

  bool CopyObject(const std::string& src_bucket,
                  const std::string& src_key,
                  const std::string& dst_bucket,
                  const std::string& dst_key) const;

  bool DeleteObject(const std::string& bucket,
                    const std::string& key) const;

  std::string GetPublicUrl(const std::string& bucket,
                           const std::string& key) const;

  std::string GeneratePresignedUrl(const std::string& bucket,
                                   const std::string& key,
                                   uint64_t expiry_seconds = 3600);

  bool CreateBucket(const std::string& bucket) const;

 private:
  Aws::S3::S3Client client_;
  std::string endpoint_;
  std::string region_;
};

}  // namespace s3_gateway

#endif  // S3_GATEWAY_S3_CLIENT_H_
