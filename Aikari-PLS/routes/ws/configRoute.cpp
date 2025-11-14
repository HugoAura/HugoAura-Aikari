#include "./configRoute.h"

#include <Aikari-Launcher-Public/constants/ws/errorTemplates.h>
#include <Aikari-Launcher-Public/constants/ws/errors.h>
#include <Aikari-PLS/types/constants/ws/config.h>
#include <Aikari-Shared/utils/string.h>

#include "../../lifecycle.h"

namespace launcherWsConstants = AikariLauncherPublic::Constants::WebSocket;
namespace plsWsConstants = AikariPLS::Types::Constants::WebSocket;

namespace AikariPLS::Routes::WebSocket::Config
{
    namespace Rules
    {
        static AikariShared::Types::InterThread::SubToMainWebSocketMessageCore
        _impl_GET_CONFIG()
        {
            auto& sharedIns =
                AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();
            const auto* configMgr = sharedIns.getPtr(
                &AikariPLS::Types::Lifecycle::PLSSharedIns::configMgr
            );
            if (configMgr != nullptr)
            {
                AikariPLS::Types::Config::PLSConfig curConfig =
                    *std::atomic_load(&configMgr->config);
                return { .code = 0, .success = true, .data = curConfig.rules };
            }
            else
            {
                return {
                    .code = launcherWsConstants::Errors::Codes::GENERIC_FAILURE,
                    .success = false,
                    .data = { { "message",
                                "PLS Config Manager offline, please check "
                                "Aikari logs for further information" } }
                };
            }
        };

        static AikariShared::Types::InterThread::SubToMainWebSocketMessageCore
        _impl_UPDATE_CONFIG(const nlohmann::json& clientData)
        {
            if (!clientData.contains("key") || !clientData.contains("value") ||
                !clientData.contains("affiliated"))
            {
                return { .code =
                             launcherWsConstants::Errors::Codes::INVALID_ARG,
                         .success = false,
                         .data = { { "message", "Invalid arguments" } } };
            }
            auto& sharedIns =
                AikariPLS::Lifecycle::PLSSharedInsManager::getInstance();
            auto* configMgr = sharedIns.getPtr(
                &AikariPLS::Types::Lifecycle::PLSSharedIns::configMgr
            );
            auto* ruleMgr = sharedIns.getPtr(
                &AikariPLS::Types::Lifecycle::PLSSharedIns::ruleMgr
            );
            if (configMgr != nullptr && ruleMgr != nullptr)
            {
                AikariPLS::Types::Config::PLSConfig curFullConfig =
                    *std::atomic_load(&configMgr->config);
                nlohmann::json& ruleConfigInConfigMgr = curFullConfig.rules;
                std::string targetAsJsonPointerStr = std::format(
                    "/{}",
                    AikariShared::Utils::String::replaceAll(
                        clientData["key"], ".", "/"
                    )
                );
                const nlohmann::json::json_pointer targetAsJsonPointer(
                    targetAsJsonPointerStr
                );
                nlohmann::json* targetConfigItem;
                try
                {
                    targetConfigItem =
                        &ruleConfigInConfigMgr.at(targetAsJsonPointer);
                }
                catch (...)
                {
                    return {
                        .code =
                            launcherWsConstants::Errors::Codes::RUNTIME_FAILURE,
                        .success = false,
                        .data = { { "message", "Invalid \"key\" provided" } }
                    };
                }
                *targetConfigItem = clientData["value"].get<nlohmann::json>();
                auto newConfigPtr =
                    std::make_shared<AikariPLS::Types::Config::PLSConfig>(
                        curFullConfig
                    );
                configMgr->configEditLock.lock();
                std::atomic_store(&configMgr->config, newConfigPtr);
                configMgr->configEditLock.unlock();
                if (!clientData.contains("write") ||
                    clientData["write"].get<bool>())
                {
                    configMgr->writeConfig();
                };
                if (ruleMgr->configKeyAssociationMap.contains(
                        clientData.at("affiliated")
                    ))
                {
                    nlohmann::json::json_pointer configKeyAsJsonPointer(
                        std::format(
                            "/{}",
                            AikariShared::Utils::String::replaceAll(
                                clientData["affiliated"], ".", "/"
                            )
                        )
                    );
                    nlohmann::json newConfigPart =
                        ruleConfigInConfigMgr[configKeyAsJsonPointer];
                    for (const auto* perRawPtr :
                         ruleMgr->configKeyAssociationMap.at(
                             clientData["affiliated"]
                         ))
                    {
                        if (perRawPtr == nullptr)
                            continue;
                        if (auto* perRewriteRule = perRawPtr->get();
                            perRewriteRule != nullptr)
                        {
                            perRewriteRule->onConfigUpdate(
                                newConfigPart
                            );  // safe, onConfigUpdate will copy newConfigPart
                        }
                    }
                }
                return { .code = 0,
                         .success = true,
                         .data = {
                             { "message", "Config successfully updated" } } };
            }
            else
            {
                return {
                    .code = launcherWsConstants::Errors::Codes::GENERIC_FAILURE,
                    .success = false,
                    .data = { { "message",
                                "PLS Config manager or Rule Manager "
                                "offline, please check Aikari logs for further "
                                "information" } }
                };
            }
        };
    }  // namespace Rules

    AikariShared::Types::InterThread::SubToMainWebSocketMessageCore
    handleConfigMsg(
        const AikariShared::Types::InterThread::MainToSubWebSocketMessageCore&
            clientDataIncoming,
        const std::vector<std::string>& methodVec
    )
    {
        const std::string& subMethod = methodVec.at(1);
        const std::string& fullMethod = clientDataIncoming.method;
        AikariShared::Types::InterThread::SubToMainWebSocketMessageCore result;
        result.code = launcherWsConstants::Errors::Codes::UNEXPECTED_ERROR;
        result.success = false;
        result.data = { { "message", "Method not found" },
                        { "diagnoseId",
                          launcherWsConstants::Errors::METHOD_NOT_FOUND } };

        if (subMethod == plsWsConstants::Config::Rules::_PREFIX)
        {
            if (fullMethod == plsWsConstants::Config::Rules::GET_RULE_CONFIG)
            {
                return Rules::_impl_GET_CONFIG();
            }
            else if (fullMethod ==
                     plsWsConstants::Config::Rules::UPDATE_RULE_CONFIG)
            {
                return Rules::_impl_UPDATE_CONFIG(clientDataIncoming.data);
            }
        }

        return result;
    };
}  // namespace AikariPLS::Routes::WebSocket::Config
