#include "wsServer.h"

#include <ixwebsocket/IXSocketTLSOptions.h>

#include <chrono>
#include <nlohmann/json.hpp>
#include <random>

#include "../infrastructure/registry.h"
#include "../lifecycle.h"
#include "../middleware/wsAuthHandler.h"
#include "../utils/cryptoUtils.h"
#include "../utils/windowsUtils.h"

namespace winUtils = AikariUtils::WindowsUtils;

namespace AikariLauncherComponents::AikariWebSocketServer
{
// ↓ public
int MainWSServer::launchWssServer()
{
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
                [this, connectionState, webSocketWeak](
                    const ix::WebSocketMessagePtr& msg
                ) { this->handleOnMsg(webSocketWeak, connectionState, msg); }
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
    LOG_INFO(std::format(
        "WebSocket server listening on wss://{}:{}",
        this->wsBindAddr,
        this->wsPort
    ));
    this->writeRegInfo();
    return 0;
}

// ↓ public
bool MainWSServer::tryLaunchWssServer()
{
    for (int8_t i = 0; i < this->maxStartupRetries; ++i)
    {
        LOG_INFO(std::format(
            "Trying to start Aikari WebSocket server ({} / {})",
            i + 1,
            this->maxStartupRetries
        ));
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
    this->wsStates->wsSrvIns->stop();
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
        auto webSocketIns = webSocketWeak.lock();

        auto authStatus =
            AikariLauncherMiddlewares::WebSocket::handleClientAuth(
                msg->openInfo.uri, this->authToken
            );

        if (authStatus !=
            AikariTypes::middleware::websocket::WEBSOCKET_AUTH_STATUS::PASSED)
        {
            const nlohmann::json deniedRep = {
                { "code", -1 },
                { "type", "basic.auth.reportAuthStatus" },
                { "success", false },
                { "data", { { "authStatus", "denied" } } }
            };
            webSocketIns->send(deniedRep.dump());
            LOG_WARN(std::format(
                "Client {} failed to complete authentication, closing "
                "connection...",
                connectionState->getId()
            ));
            webSocketIns->close();
            return;
        }

        LOG_INFO(std::format(
            "New client {} connected from {}:{}, welcome.",
            connectionState->getId(),
            connectionState->getRemoteIp(),
            connectionState->getRemotePort()
        ));

        {
            std::lock_guard<std::mutex> lock(this->wsStates->clientLsMutex);
            this->wsStates->clients.emplace_back(webSocketWeak);
        }
    }
    else if (msg->type == ix::WebSocketMessageType::Close)
    {
        LOG_INFO(std::format(
            "Client {} disconnected, bye.", connectionState->getId()
        ));
        // No need for clean up, powered by weak_ptr
    }
    else if (msg->type == ix::WebSocketMessageType::Error)
    {
        LOG_WARN(std::format(
            "Unexpected error occurred between server and client {}",
            connectionState->getId()
        ));
    }
    else if (msg->type == ix::WebSocketMessageType::Message)
    {
#ifdef _DEBUG
        LOG_TRACE(std::format(
            "⏬ New WebSocket message from client {} received:\r\n{}",
            connectionState->getId(),
            msg->str
        ));
#endif
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
        winUtils::StringToWstring(this->wsBindAddr),
        std::to_wstring(this->wsPort),
        winUtils::StringToWstring(this->authToken)
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
