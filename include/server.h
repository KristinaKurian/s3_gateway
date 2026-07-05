#ifndef S3_GATEWAY_SERVER_H_
#define S3_GATEWAY_SERVER_H_

#include <atomic>
#include <memory>

#include <httplib.h>

#include "config.h"
#include "file_registry.h"
#include "metrics.h"
#include "s3_client.h"

namespace s3_gateway {

class Server {
 public:
  explicit Server(Config config);

  void Run();
  void Stop();

 private:
  void RegisterRoutes();
  void SyncRegistry();
  void LoadRegistryManifests();
  void BackfillRegistryFromBuckets();
  void ReconcileRegistryWithBuckets();
  void PersistRegistryManifests();
  std::optional<FileEntry> ResolveMissingFile(const std::string& key);
  std::optional<FileEntry> ResolveRoute(const std::string& route);
  void RefreshMetrics();

  void HandleStatus(const httplib::Request& req, httplib::Response& res);
  void HandleReadiness(const httplib::Request& req, httplib::Response& res);
  void HandleHealthcheck(const httplib::Request& req, httplib::Response& res);
  void HandleMetrics(const httplib::Request& req, httplib::Response& res);
  void HandleList(const httplib::Request& req, httplib::Response& res);
  void HandleUpdate(const httplib::Request& req, httplib::Response& res);
  void HandleRelocate(const httplib::Request& req, httplib::Response& res);
  void HandleFileDownload(const httplib::Request& req, httplib::Response& res);

  Config config_;
  httplib::Server http_;
  std::unique_ptr<S3Client> s3_;
  std::unique_ptr<FileRegistry> registry_;
  std::unique_ptr<Metrics> metrics_;
  std::atomic_bool ready_{false};
};

}  // namespace s3_gateway

#endif 
