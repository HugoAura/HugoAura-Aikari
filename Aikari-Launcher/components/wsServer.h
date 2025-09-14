#pragma once
#pragma comment(lib, "crypt32")

#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Launcher-Private/types/components/wsTypes.h>
#include <Aikari-Shared/infrastructure/queue/PoolQueue.hpp>
#include <Aikari-Shared/infrastructure/queue/SinglePointMessageQueue.hpp>
#include <filesystem>
#include <ixwebsocket/IXWebSocketServer.h>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace AikariLauncherComponents::AikariWebSocketServer
{
    struct WebSocketStates
    {
        std::unique_ptr<ix::WebSocketServer> wsSrvIns;
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

        ~MainWSServer();

        int launchWssServer();
        bool tryLaunchWssServer();
        void waitWssServer();
        void stopWssServer();
        void pushRetQueue(
            AikariTypes::Components::WebSocket::ServerWSTaskRet& ret
        );

       private:
        std::filesystem::path wssCertPath;
        std::filesystem::path wssKeyPath;
        int8_t maxStartupRetries = 5;
        std::shared_ptr<WebSocketStates> wsStates;
        std::unique_ptr<
            AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariTypes::Components::WebSocket::ServerWSTaskRet>>
            retMsgQueue;
        std::unique_ptr<
            AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariTypes::Components::WebSocket::ClientWSTask>>
            inputMsgQueue;

        std::unique_ptr<std::jthread> retMsgWorkerThread;
        std::unique_ptr<std::jthread> inputMsgWorkerThread;

        std::unique_ptr<AikariShared::Infrastructure::MessageQueue::PoolQueue<
            AikariTypes::Components::WebSocket::ClientWSTask>>
            threadPool;

        size_t threadCount = 4;

        bool isStopped = false;

        void retMsgWorker();
        void inputMsgWorker();
        void handleOnMsg(
            std::weak_ptr<ix::WebSocket> webSocketWeak,
            ix::ConnectionState* connectionState,
            const ix::WebSocketMessagePtr& msg
        );
        bool writeRegInfo();
        static std::string genAuthToken();
        static int genRandomPort();
    };
}  // namespace AikariLauncherComponents::AikariWebSocketServer
