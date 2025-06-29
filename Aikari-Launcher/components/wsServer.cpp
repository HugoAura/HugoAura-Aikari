#include "pch.h"

#include "wsServer.h"

#include <Aikari-Launcher-Private/types/components/wsTypes.h>
#include <Aikari-Shared/infrastructure/MessageQueue.hpp>
#include <Aikari-Shared/utils/string.h>
#include <chrono>
#include <ixwebsocket/IXSocketTLSOptions.h>
#include <nlohmann/json.hpp>
#include <random>

#include "../infrastructure/registry.h"
#include "../lifecycle.h"
#include "../middleware/wsAuthHandler.h"
#include "../utils/cryptoUtils.h"
#include "wsMsgHandler.h"

namespace winStringUtils = AikariShared::utils::string;
namespace messageQueue = AikariShared::infrastructure::MessageQueue;

typedef messageQueue::SinglePointMessageQueue<
    AikariTypes::components::websocket::ClientWSTask>
    InputMsgQueue;

typedef messageQueue::SinglePointMessageQueue<
    AikariTypes::components::websocket::ServerWSTaskRet>
    RetMsgQueue;

namespace AikariLauncherComponents::AikariWebSocketServer
{
// ↓ public
int MainWSServer::launchWssServer()
{
    LOG_INFO("Creating ws message queue...");
    this->inputMsgQueue = std::make_shared<InputMsgQueue>();
    this->retMsgQueue = std::make_shared<RetMsgQueue>();

    this->wsStates->wsSrvIns =
        std::make_shared<ix::WebSocketServer>(this->wsPort, this->wsBindAddr);
    LOG_INFO("Created WebSocket server instance.");

    ix::SocketTLSOptions tlsOptions;
    tlsOptions.certFile = this->wssCertPath.string();
    tlsOptions.keyFile = this->wssKeyPath.string();
    tlsOptions.tls = true;
    tlsOptions.disable_hostname_validation = true;
    tlsOptions.caFile = "NONE";  // disable peer verification

    this->wsStates->wsSrvIns->setTLSOptions(tlsOptions);
    LOG_DEBUG("WebSocket server TLS options set up.");

    // End of srvIns cfg
    this->wsStates->wsSrvIns->setOnConnectionCallback(
        [this](
            std::weak_ptr<ix::WebSocket> webSocketWeak,
            std::shared_ptr<ix::ConnectionState> connectionState
        )
        {
            std::shared_ptr<ix::WebSocket> webSocketIns = webSocketWeak.lock();

            if (!webSocketIns)
                return;

            webSocketIns->setOnMessageCallback(
                [this,
                 connectionState,
                 webSocketWeak](const ix::WebSocketMessagePtr& msg)
                {
                    this->handleOnMsg(webSocketWeak, connectionState, msg);
                }
            );
        }
    );

    auto res = this->wsStates->wsSrvIns->listen();

    if (!res.first)
    {
        LOG_ERROR("Failed to start Aikari ws server, error: " + res.second);
        return -1;
    }

    this->wsStates->wsSrvIns->start();
    LOG_INFO("Aikari WebSocket server started!");
    LOG_INFO(
        "WebSocket server listening on wss://{}:{}",
        this->wsBindAddr,
        this->wsPort
    );
    this->writeRegInfo();
    LOG_INFO("Launching WebSocket message workers...");
    this->inputMsgWorkerThread =
        std::make_shared<std::jthread>(&MainWSServer::inputMsgWorker, this);
    this->retMsgWorkerThread =
        std::make_shared<std::jthread>(&MainWSServer::retMsgWorker, this);
    return 0;
}

// ↓ public
bool MainWSServer::tryLaunchWssServer()
{
    for (int8_t i = 0; i < this->maxStartupRetries; ++i)
    {
        LOG_INFO(
            "Trying to start Aikari WebSocket server ({} / {})",
            i + 1,
            this->maxStartupRetries
        );
        int res = this->launchWssServer();
        if (res == 0)
        {
            LOG_INFO("Successfully launched Aikari ws server.");
            return true;
        }
        else
        {
            LOG_WARN("Error starting server, retrying...");
            this->wsPort = genRandomPort();
            LOG_DEBUG("Next port: " + std::to_string(this->wsPort));
        }
    }
    return false;
}

// ↓ public
void MainWSServer::waitWssServer()
{
    this->wsStates->wsSrvIns->wait();
}

// ↓ public
void MainWSServer::stopWssServer()
{
    this->inputMsgQueue->push({ .clientId = "-1" });
    this->retMsgQueue->push({ .clientId = "-1" });
    this->inputMsgWorkerThread->join();
    this->retMsgWorkerThread->join();
    for (auto& perThread : this->msgProcThreads)
    {
        if (perThread->joinable())
        {
            perThread->join();
        }
    }
    this->wsStates->wsSrvIns->stop();
}

// ↓ public
void MainWSServer::pushRetQueue(
    AikariTypes::components::websocket::ServerWSTaskRet& ret
)
{
    this->retMsgQueue->push(std::move(ret));
    return;
}

// ↓ private
void MainWSServer::retMsgWorker()
{
    LOG_DEBUG("WebSocket reply worker thread started.");
    try
    {
        while (true)
        {
            auto ret = this->retMsgQueue->pop();
            if (ret.clientId == "-1")
            {
                break;
            }
#ifdef _DEBUG
            LOG_DEBUG("retMsgWorker: Sending data: " + ret.result.data.dump());
#endif
            nlohmann::json repJson;
            repJson = { { "success", ret.result.success },
                        { "eventId", ret.result.eventId },
                        { "code", ret.result.code },
                        { "data", ret.result.data } };
            if (ret.isBroadcast)
            {
                auto itr = this->wsStates->clients.begin();
                while (itr != this->wsStates->clients.end())
                {
                    if (auto perClientLocked = itr->second.lock())
                    {
                        perClientLocked->send(repJson.dump());
                        itr++;
                    }
                    else
                    {
                        itr = this->wsStates->clients.erase(itr);
                    }
                }
            }
            else
            {
                auto client = this->wsStates->clients.find(ret.clientId);
                if (client == this->wsStates->clients.end())
                {
                    LOG_WARN(
                        "Cannot reply to client {}: Client disconnected",
                        ret.clientId
                    );
                    return;
                }
                auto clientLocked = client->second.lock();
                if (clientLocked != NULL)
                {
                    clientLocked->send(repJson.dump());
                }
                else
                {
                    this->wsStates->clients.erase(client);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        LOG_CRITICAL(
            "Critical error occurred in retMsgWorker thread: {}", e.what()
        );
    }
}

// ↓ private
void MainWSServer::inputMsgWorker()
{
    LOG_DEBUG("WebSocket input message worker thread started.");
    try
    {
        while (true)
        {
            auto task = this->inputMsgQueue->pop();
            if (task.clientId == "-1")
            {
                break;
            }
            auto perThread = std::make_shared<std::jthread>(
                AikariLauncherComponents::AikariWebSocketHandler::handleTask,
                task,
                this->retMsgQueue
            );
            this->msgProcThreads.emplace_back(perThread);
        }
    }
    catch (const std::exception& e)
    {
        LOG_CRITICAL(
            "Critical error occurred in inputMsgWorker thread: {}", e.what()
        );
    }
}

// ↓ private
void MainWSServer::handleOnMsg(
    std::weak_ptr<ix::WebSocket> webSocketWeak,
    std::shared_ptr<ix::ConnectionState> connectionState,
    const ix::WebSocketMessagePtr& msg
)
{
    if (msg->type == ix::WebSocketMessageType::Open)
    {
        const std::string& clientId = connectionState->getId();

        auto webSocketIns = webSocketWeak.lock();

        auto authStatus =
            AikariLauncherMiddlewares::WebSocket::handleClientAuth(
                msg->openInfo.uri, this->authToken
            );

        if (authStatus !=
            AikariTypes::middleware::websocket::WEBSOCKET_AUTH_STATUS::PASSED)
        {
            static const nlohmann::json deniedRep = {
                { "code", -1 },
                { "type", "basic.auth.reportAuthStatus" },
                { "success", false },
                { "data", { { "authStatus", "denied" } } }
            };
            webSocketIns->send(deniedRep.dump());
            LOG_WARN(
                "Client {} failed to complete authentication, closing "
                "connection...",
                clientId
            );
            webSocketIns->close();
            return;
        }

        LOG_INFO(
            "New client {} connected from {}:{}, welcome.",
            clientId,
            connectionState->getRemoteIp(),
            connectionState->getRemotePort()
        );

        {
            std::lock_guard<std::mutex> lock(this->wsStates->clientLsMutex);
            this->wsStates->clients[clientId] = webSocketWeak;
        }
    }
    else if (msg->type == ix::WebSocketMessageType::Close)
    {
        const std::string& clientId = connectionState->getId();
        LOG_INFO("Client {} disconnected, bye.", clientId);
        this->wsStates->clients.erase(clientId);
    }
    else if (msg->type == ix::WebSocketMessageType::Error)
    {
        LOG_WARN(
            "Unexpected error occurred between server and client {}",
            connectionState->getId()
        );
    }
    else if (msg->type == ix::WebSocketMessageType::Message)
    {
#ifdef _DEBUG
        LOG_TRACE(
            "⏬ New WebSocket message from client {} received:\r\n{}",
            connectionState->getId(),
            msg->str
        );
#endif

        const std::string& clientId = connectionState->getId();
        nlohmann::json clientMsgJson;
        AikariTypes::components::websocket::ClientWSMsg clientMsg;
        try
        {
            clientMsgJson = nlohmann::json::parse(msg->str);
            clientMsg.module = clientMsgJson.at("module");
            clientMsg.method = clientMsgJson.at("method");
            clientMsg.eventId = clientMsgJson.at("eventId");
            clientMsg.data = clientMsgJson.at("data");
        }
        catch (const nlohmann::json::exception& e)
        {
            LOG_ERROR(
                "Client {} sent invalid data, error: {}", clientId, e.what()
            );

            auto webSocketIns = webSocketWeak.lock();
            if (webSocketIns)
            {
                static const nlohmann::json invalidDataJson = {
                    { "code", -1 },
                    { "success", false },
                    { "eventId", "UNKNOWN" },
                    { "data", { { "message", "Invalid json format" } } }
                };
                webSocketIns->send(invalidDataJson.dump());
            }
            return;
        }

        AikariTypes::components::websocket::ClientWSTask taskIns;
        taskIns.content = clientMsg;
        taskIns.clientId = clientId;
        this->inputMsgQueue->push(std::move(taskIns));
    }
};

// ↓ private
bool MainWSServer::writeRegInfo()
{
    auto& lifecycleMgr = AikariLifecycle::AikariStatesManager::getInstance();
    std::shared_ptr<AikariRegistry::RegistryManager> registryManagerPtr =
        lifecycleMgr
            .getVal(&AikariTypes::global::lifecycle::GlobalLifecycleStates::
                        sharedIns)
            .registryManagerIns;

    if (!registryManagerPtr)
    {
        LOG_ERROR("Failed to write ws info to registry: registryMgrPtr is NULL."
        );
        return false;
    }

    std::wstring targetKey = registryManagerPtr->aikariRootKey + L"\\RPC";
    std::vector<std::wstring> entries = { L"wsHost", L"wsPort", L"authToken" };
    std::vector<std::wstring> contents = {
        winStringUtils::StringToWstring(this->wsBindAddr),
        std::to_wstring(this->wsPort),
        winStringUtils::StringToWstring(this->authToken)
    };

    int writeKeyResult =
        registryManagerPtr->writeRegKeySz(targetKey, entries, contents);

    if (writeKeyResult == -1)
    {
        LOG_ERROR(
            "Failed to write ws info to registry: writeRegKeySz call failed."
        );
        return false;
    }
    else
    {
        return true;
    }
};

// ↓ private
std::string MainWSServer::genAuthToken()
{
    auto& lifecycleMgr = AikariLifecycle::AikariStatesManager::getInstance();

    size_t tokenSize = 32;
    std::string authTokenHex =
        AikariUtils::cryptoUtils::genRandomHexSecure(tokenSize);

    return authTokenHex;
};

// ↓ private
int MainWSServer::genRandomPort()
{
    std::mt19937 generator(static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    ));
    std::uniform_int_distribution<int> dist(10000, 65535);

    int randomPort = dist(generator);
    return randomPort;
};
}  // namespace AikariLauncherComponents::AikariWebSocketServer
