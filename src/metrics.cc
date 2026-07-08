#include "metrics.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>

#include <prometheus/text_serializer.h>

namespace s3_gateway {

namespace {

constexpr auto kWeekWindow = std::chrono::days{7};
constexpr auto kMonthWindow = std::chrono::days{31};

std::optional<std::chrono::system_clock::time_point> ParseIso8601(
    const std::string& timestamp) {
  if (timestamp.size() < 19) {
    return std::nullopt;
  }

  std::tm tm{};
  std::istringstream stream(timestamp.substr(0, 19));
  if (timestamp.size() > 19 && timestamp[19] != 'Z') {
    return std::nullopt;
  }
  stream >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

  if (stream.fail()) {
    return std::nullopt;
  }

  using namespace std::chrono;

  const year y{tm.tm_year + 1900};
  const month m{static_cast<unsigned>(tm.tm_mon + 1)};
  const day d{static_cast<unsigned>(tm.tm_mday)};

  const year_month_day ymd{y / m / d};

  if (!ymd.ok()) {
    return std::nullopt;
  }

  const auto time =
      sys_days{ymd}
      + hours{tm.tm_hour}
      + minutes{tm.tm_min}
      + seconds{tm.tm_sec};

  return time_point_cast<std::chrono::system_clock::duration>(time);
}
}

Metrics::Metrics()
    : registry_(std::make_shared<prometheus::Registry>()),
      volume_files_family_(
          prometheus::BuildGauge()
              .Name("s3_gateway_volume_files")
              .Help("Current number of known files by bucket.")
              .Register(*registry_)),
      duplicate_files_(
          prometheus::BuildGauge()
              .Name("s3_gateway_duplicate_files")
              .Help(
                  "Number of file keys that exist in more than one S3 "
                  "bucket.")
              .Register(*registry_)
              .Add({})),
      uploads_per_week_(
          prometheus::BuildGauge()
              .Name("s3_gateway_uploads_per_week")
              .Help(
                  "Known uploaded file versions modified during the last 7 "
                  "days.")
              .Register(*registry_)
              .Add({})),
      uploads_per_month_(
          prometheus::BuildGauge()
              .Name("s3_gateway_uploads_per_month")
              .Help(
                  "Known uploaded file versions modified during the last 31 "
                  "days.")
              .Register(*registry_)
              .Add({})) {}

void Metrics::ObserveFiles(const std::vector<FileEntry>& entries) {
  {
    std::lock_guard lock(mu_);
    std::unordered_map<std::string, int> counts;
    int week = 0;
    int month = 0;
    const auto now = std::chrono::system_clock::now();

    for (const auto& entry : entries) {
      ++counts[entry.bucket_name];
      const auto modified_at = ParseIso8601(entry.last_modified);
      if (!modified_at) {
        continue;
      }
      if (*modified_at <= now && *modified_at >= now - kWeekWindow) {
        ++week;
      }

      if (*modified_at <= now && *modified_at >= now - kMonthWindow) {
        ++month;
      }
    }

    for (auto& [bucket, gauge] : volume_files_gauges_) {
      gauge->Set(0);
    }
    for (const auto& [bucket, count] : counts) {
      auto it = volume_files_gauges_.find(bucket);
      if (it == volume_files_gauges_.end()) {
        auto& gauge = volume_files_family_.Add({{"bucket", bucket}});
        volume_files_gauges_[bucket] = &gauge;
        gauge.Set(count);
      } else {
        it->second->Set(count);
      }
    }

    uploads_per_week_.Set(week);
    uploads_per_month_.Set(month);
  }
}

void Metrics::SetDuplicateFiles(int count) {
  {
    std::lock_guard lock(mu_);
    duplicate_files_.Set(std::max(0, count));
  }
}

std::string Metrics::Render() const {
  std::ostringstream output;
  prometheus::TextSerializer serializer;
  serializer.Serialize(output, registry_->Collect());
  return output.str();
}

}  // namespace s3_gateway
