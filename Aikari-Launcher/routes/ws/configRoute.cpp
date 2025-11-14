#include "./configRoute.h"

#include <Aikari-Launcher-Public/constants/ws/config.h>
#include <Aikari-Launcher-Public/constants/ws/errorTemplates.h>
#include <Aikari-Launcher-Public/constants/ws/errors.h>
#include <Aikari-Shared/utils/string.h>

#include "../../components/config.h"
#include "../../lifecycle.h"
#include "../../utils/lifecycleUtils.h"

namespace wsConstants = AikariLauncherPublic::Constants::WebSocket;

namespace AikariLauncherRoutes::WebSocket::Config
{
    namespace Actions
    {
        static AikariLauncherPublic::Types::Components::WebSocket::ServerWSRep
        _impl_GET_CONFIG()
        {
            auto& sharedIns =
                AikariLifecycle::AikariSharedInstances::getInstance();
            auto* configMgr =
                sharedIns.getPtr(&AikariTypes::Global::Lifecycle::
                                     SharedInstances::configManagerIns);
            if (configMgr != nullptr)
            {
                AikariTypes::Config::AikariConfig config(
                    *std::atomic_load(&configMgr->config)
                );
                nlohmann::json jsonifyConfig;
                AikariTypes::Config::to_json(jsonifyConfig, config);
                return { .code = 0,
                         .success = true,
                         .data = std::move(jsonifyConfig) };
            }
            else
            {
                return { .code = AikariLauncherPublic::Constants::WebSocket::
                             Errors::Codes::GENERIC_FAILURE,
                         .success = false,
                         .data = {
                             { "message", "Config manager internal error" } } };
            }
        }

        static AikariLauncherPublic::Types::Components::WebSocket::ServerWSRep
        _impl_UPDATE_CONFIG(nlohmann::json clientDataIn)
        {
            if (!clientDataIn.contains("key") ||
                !clientDataIn.contains("value"))
            {
                return wsConstants::Errors::Templates::INVALID_ARG;
            }
            auto& sharedIns =
                AikariLifecycle::AikariSharedInstances::getInstance();
            auto* configMgr =
                sharedIns.getPtr(&AikariTypes::Global::Lifecycle::
                                     SharedInstances::configManagerIns);
            if (configMgr != nullptr)
            {
                AikariTypes::Config::AikariConfig config(
                    *std::atomic_load(&configMgr->config)
                );
                nlohmann::json jsonifyConfig;
                AikariTypes::Config::to_json(jsonifyConfig, config);
                nlohmann::json* curTarget = &jsonifyConfig;
                try
                {
                    auto keyVec = AikariShared::Utils::String::split(
                        clientDataIn["key"].get<std::string>(), '.'
                    );
                    for (const auto& key : keyVec)
                    {
                        curTarget = &(curTarget->at(key));
                    }
                    *curTarget = clientDataIn["value"].get<nlohmann::json>();
                }
                catch (...)
                {
                    return { .code = wsConstants::Errors::Codes::INVALID_ARG,
                             .success = false,
                             .data = {
                                 { "message",
                                   "Invalid \"key\" argument provided" } } };
                }
                AikariTypes::Config::AikariConfig newConfig;
                try
                {
                    AikariTypes::Config::from_json(jsonifyConfig, newConfig);
                }
                catch (const nlohmann::json::type_error& err)
                {
                    return { .code =
                                 wsConstants::Errors::Codes::RUNTIME_FAILURE,
                             .success = false,
                             .data = { { "message", err.what() } } };
                }
                try
                {
                    auto newConfigPtr =
                        std::make_shared<AikariTypes::Config::AikariConfig>(
                            newConfig
                        );
                    configMgr->configEditLock.lock();
                    std::atomic_store(&configMgr->config, newConfigPtr);
                    configMgr->configEditLock.unlock();
                }
                catch (const std::exception& err)
                {
                    configMgr->configEditLock.unlock();
                    return { .code =
                                 wsConstants::Errors::Codes::RUNTIME_FAILURE,
                             .success = false,
                             .data = {
                                 { "message",
                                   std::format(
                                       "Error updating config manager data: {}",
                                       err.what()
                                   ) } } };
                }
                if (!clientDataIn.contains("write") ||
                    clientDataIn["write"].get<bool>())
                {
                    configMgr->writeConfig();
                }
                return { .code = 0,
                         .success = true,
                         .data = { { "message",
                                     "Successfully updated config file" } } };
            }
            else
            {
                return { .code = wsConstants::Errors::Codes::GENERIC_FAILURE,
                         .success = false,
                         .data = {
                             { "message", "Config manager internal error" } } };
            }
        }
    }  // namespace Actions

    AikariLauncherPublic::Types::Components::WebSocket::ServerWSRep
    handleConfigMethods(
        const AikariLauncherPublic::Types::Components::WebSocket::ClientWSMsg&
            clientDataIncoming,
        const std::vector<std::string>& methods
    )
    {
        const std::string& subMethod = methods.at(1);
        const std::string& fullMethod = clientDataIncoming.method;
        AikariLauncherPublic::Types::Components::WebSocket::ServerWSRep result;
        result.code = wsConstants::Errors::Codes::UNEXPECTED_ERROR;

        if (subMethod == wsConstants::Config::Actions::_PREFIX)
        {
            if (fullMethod == wsConstants::Config::Actions::GET_CONFIG)
            {
                return Actions::_impl_GET_CONFIG();
            }
            else if (fullMethod == wsConstants::Config::Actions::UPDATE_CONFIG)
            {
                return Actions::_impl_UPDATE_CONFIG(clientDataIncoming.data);
            }
        }

        return wsConstants::Errors::Templates::METHOD_NOT_FOUND;
    }
}
// namespace AikariLauncherRoutes::WebSocket::Config