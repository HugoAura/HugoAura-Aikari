#include "pch.h"

#include "networkRoute.h"

#include <Aikari-Launcher-Public/constants/itcCtrl/errorTemplates.h>
#include <Aikari-Launcher-Public/constants/itcCtrl/errors.h>
#include <Aikari-Launcher-Public/constants/itcCtrl/network.h>
#include <Aikari-Shared/infrastructure/loggerMacro.h>

#include "infrastructure/filesystem.h"
#include "lifecycle.h"
#include "utils/sslUtils.h"

namespace itcTypes = AikariShared::Types::InterThread;
namespace itcConstants = AikariLauncherPublic::Constants::InterThread;

namespace AikariLauncherRoutes::InterThread::Network
{
    namespace TLS
    {
        static nlohmann::json _genTlsCert(const nlohmann::json& clientData)
        {
            std::string baseDir = clientData.value("baseDir", "");
            std::filesystem::path targetPath;
            if (baseDir == "")
            {
                auto& sharedIns =
                    AikariLifecycle::AikariSharedInstances::getInstance();
                auto* fsMgr =
                    sharedIns.getPtr(&AikariTypes::global::lifecycle::
                                         SharedInstances::fsManagerIns);
                targetPath = fsMgr->aikariConfigDir / "certs";
            }
            else
            {
                targetPath = std::filesystem::path(baseDir);
            }

            std::string hostname;
            std::string identifier;
            try
            {
                hostname = clientData.at("hostname");
                identifier = clientData.at("identifier");
            }
            catch (const std::exception& err)
            {
                LOG_ERROR("Failed to parse ctrl data: ", err.what());
                return { { "success", false },
                         { "message", err.what() },
                         { "diagnoseCode",
                           itcConstants::Errors::INVALID_ARG } };
            }

            try
            {
                AikariUtils::sslUtils::genEC256TlsCert(
                    targetPath, hostname, identifier
                );
            }
            catch (const std::exception& err)
            {
                LOG_ERROR(
                    "Unexpected error occurred generating TLS cert: {}",
                    err.what()
                );
                return { { "success", false },
                         { "message", err.what() },
                         { "diagnoseCode",
                           itcConstants::Errors::LAMBDA_CALL_FAILED } };
            }

            return { { "success", true },
                     { "message", "Successfully generated" } };
        }
    }  // namespace TLS

    itcTypes::MainToSubControlReplyMessage handleNetworkCtrlMessage(
        const itcTypes::SubToMainControlMessage& incoming,
        const std::vector<std::string>& routes
    )
    {
        const std::string& subRoute = routes.at(1);
        const std::string& fullRoute = incoming.method;
        itcTypes::MainToSubControlReplyMessage result = {
            .eventId = incoming.eventId
        };

        if (subRoute == itcConstants::Network::TLS::_PREFIX)
        {
            if (fullRoute == itcConstants::Network::TLS::GEN_TLS_CERTS)
            {
                result.data = TLS::_genTlsCert(incoming.data);
            }
            else
            {
                result.data = itcConstants::Errors::Templates::ROUTE_NOT_FOUND;
            }
        }
        else
        {
            result.data = itcConstants::Errors::Templates::ROUTE_NOT_FOUND;
        }

        return result;
    };

}  // namespace AikariLauncherRoutes::InterThread::Network
