#ifndef S3_GATEWAY_METRICS_H_
#define S3_GATEWAY_METRICS_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <prometheus/gauge.h>
#include <prometheus/registry.h>

#include "file_registry.h"

namespace s3_gateway {

class Metrics {
 public:
  Metrics();

  void ObserveFiles(const std::vector<FileEntry>& entries);
  void SetDuplicateFiles(int count);
  std::string Render() const;

 private:
  std::shared_ptr<prometheus::Registry> registry_;
  prometheus::Family<prometheus::Gauge>& volume_files_family_;
  std::unordered_map<std::string, prometheus::Gauge*> volume_files_gauges_;
  prometheus::Gauge& duplicate_files_;
  prometheus::Gauge& uploads_per_week_;
  prometheus::Gauge& uploads_per_month_;
};

}  // namespace s3_gateway

#endif  // S3_GATEWAY_METRICS_H_
