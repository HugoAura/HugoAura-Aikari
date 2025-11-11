#include "wsAuthHandler.h"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <ada.h>

#include "../lifecycle.h"

namespace wsTypes = AikariTypes::Middleware::WebSocket;

namespace AikariLauncherMiddlewares::WebSocket
{
    wsTypes::WEBSOCKET_AUTH_STATUS handleClientAuth(
        const std::string& uri, const std::string& authToken
    )
    {
        auto parsedURI = ada::parse<ada::url_aggregator>(
            "http://aikari.hugoaura.local" + uri
        );

        if (!parsedURI)
        {
            LOG_WARN("Failed to parse ws uri: " + uri);
            return wsTypes::WEBSOCKET_AUTH_STATUS::DENIED;
        }

        LOG_DEBUG("Client connecting with path: {}", parsedURI->get_pathname());

        auto param = parsedURI->get_search();
        if (param.empty())
        {
            LOG_WARN("No param provided.");
            return wsTypes::WEBSOCKET_AUTH_STATUS::DENIED;
        }

        param = param.substr(1);  // 丢掉那个 "?"

        ada::url_search_params paramIns(param);
        if (!paramIns.has("auth"))
        {
            LOG_INFO("Client didn't provide auth param, denying...");
            return wsTypes::WEBSOCKET_AUTH_STATUS::DENIED;
        }

        auto authTokenOpt = paramIns.get("auth");
        std::string userAuthToken =
            authTokenOpt.has_value() ? std::string(authTokenOpt.value()) : "";

        /*
        auto& lifecycleIns =
            AikariLifecycle::AikariStatesManager::getInstance();*/

        if (userAuthToken == authToken)
        {
            LOG_DEBUG("Authenticated successfully.");
            return wsTypes::WEBSOCKET_AUTH_STATUS::PASSED;
        }
        else
        {
            LOG_WARN("Invalid auth token provided: " + userAuthToken);
            return wsTypes::WEBSOCKET_AUTH_STATUS::DENIED;
        }
    };
}  // namespace AikariLauncherMiddlewares::WebSocket
