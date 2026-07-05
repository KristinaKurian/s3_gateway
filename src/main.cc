#include <csignal>
#include <iostream>

#include <aws/core/Aws.h>

#include "config.h"
#include "server.h"

namespace {
s3_gateway::Server* g_server = nullptr;

void SignalHandler(int /*signal*/) {
  if (g_server != nullptr) {
    g_server->Stop();
  }
}
}  // namespace

int main() {
  Aws::SDKOptions options;
  Aws::InitAPI(options);

  int exit_code = 0;
  try {
    auto config = s3_gateway::Config::FromEnv();
    s3_gateway::Server server(std::move(config));

    g_server = &server;
    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    server.Run();
  } catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << std::endl;
    exit_code = 1;
  }

  Aws::ShutdownAPI(options);
  return exit_code;
}
