#pragma once

#include <Aikari-Launcher-Private/common.h>

#include <filesystem>

namespace AikariLauncherComponents::AikariWebSocketServer
{
class MainWSServer
{
   public:
    MainWSServer(std::string& wsBindAddr, int wsDefaultPort, std::filesystem::path& wssCertPath,
                 std::filesystem::path& wssKeyPath)
        : wsBindAddr(wsBindAddr), wsPort(wsDefaultPort), wssCertPath(wssCertPath), wssKeyPath(wssKeyPath) {};

    int launchWssServer();

   private:
    std::string& wsBindAddr;
    int wsPort;
    std::filesystem::path& wssCertPath;
    std::filesystem::path& wssKeyPath;
    int8_t maxStartupRetries = 5;  // 节 约 内 存 (雾
};
}  // namespace AikariLauncherComponents::AikariWebSocketServer
