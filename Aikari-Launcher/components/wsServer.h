#pragma once
#pragma comment(lib, "crypt32")

#include <Aikari-Launcher-Private/common.h>
#include <ixwebsocket/IXWebSocketServer.h>

#include <filesystem>

namespace AikariLauncherComponents::AikariWebSocketServer
{

struct WebSocketStates
{
    std::shared_ptr<ix::WebSocketServer> wsSrvIns;
    std::mutex clientLsMutex;
    std::vector<std::weak_ptr<ix::WebSocket>> clients;
};

class MainWSServer
{
   public:
    std::string wsBindAddr;
    int wsPort;
    std::string authToken;

    MainWSServer(
        std::string wsBindAddr,
        int wsDefaultPort,
        std::filesystem::path wssCertPath,
        std::filesystem::path wssKeyPath
    )
        : wsBindAddr(wsBindAddr),
          wsPort(wsDefaultPort),
          authToken(genAuthToken()),
          wssCertPath(wssCertPath),
          wssKeyPath(wssKeyPath),
          wsStates(std::make_shared<WebSocketStates>()) {};

    int launchWssServer();
    bool tryLaunchWssServer();
    void waitWssServer();
    void stopWssServer();

   private:
    std::filesystem::path wssCertPath;
    std::filesystem::path wssKeyPath;
    int8_t maxStartupRetries = 5;  // 节 约 内 存 (雾
    std::shared_ptr<WebSocketStates> wsStates;

    void handleOnMsg(
        std::weak_ptr<ix::WebSocket> webSocketWeak,
        std::shared_ptr<ix::ConnectionState> connectionState,
        const ix::WebSocketMessagePtr& msg
    );
    bool writeRegInfo();
    static std::string genAuthToken();
    static int genRandomPort();
};
}  // namespace AikariLauncherComponents::AikariWebSocketServer
