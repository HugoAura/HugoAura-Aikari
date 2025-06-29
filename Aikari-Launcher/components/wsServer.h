#pragma once
#pragma comment(lib, "crypt32")

#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Launcher-Private/types/components/wsTypes.h>
#include <Aikari-Shared/infrastructure/SinglePointMessageQueue.hpp>
#include <filesystem>
#include <ixwebsocket/IXWebSocketServer.h>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace AikariShared::infrastructure::MessageQueue
{
template <typename T>
class PoolQueue;
}

namespace AikariLauncherComponents::AikariWebSocketServer
{
struct WebSocketStates
{
    std::shared_ptr<ix::WebSocketServer> wsSrvIns;
    std::mutex clientLsMutex;
    std::unordered_map<std::string, std::weak_ptr<ix::WebSocket>> clients;
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
    void pushRetQueue(AikariTypes::components::websocket::ServerWSTaskRet& ret);

   private:
    std::filesystem::path wssCertPath;
    std::filesystem::path wssKeyPath;
    int8_t maxStartupRetries = 5;
    std::shared_ptr<WebSocketStates> wsStates;
    std::shared_ptr<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariTypes::components::websocket::ServerWSTaskRet>>
        retMsgQueue;
    std::shared_ptr<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariTypes::components::websocket::ClientWSTask>>
        inputMsgQueue;

    std::shared_ptr<std::jthread> retMsgWorkerThread;
    std::shared_ptr<std::jthread> inputMsgWorkerThread;

    std::shared_ptr<AikariShared::infrastructure::MessageQueue::PoolQueue<
        AikariTypes::components::websocket::ClientWSTask>>
        threadPool;

    int threadCount = 16;

    void retMsgWorker();
    void inputMsgWorker();
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
