#include "filesystemRoute.h"

#include <Aikari-Launcher-Public/constants/itcCtrl/errorTemplates.h>
#include <Aikari-Launcher-Public/constants/itcCtrl/errors.h>
#include <Aikari-Launcher-Public/constants/itcCtrl/filesystem.h>
#include <nlohmann/json.hpp>

#include "../../infrastructure/filesystem.h"
#include "../../lifecycle.h"

namespace itcTypes = AikariShared::Types::InterThread;
namespace itcConstants = AikariLauncherPublic::Constants::InterThread;

namespace AikariLauncherRoutes::InterThread::FileSystem
{
    namespace Base
    {
        static nlohmann::json _getDir(const nlohmann::json& param)
        {
            auto& sharedIns =
                AikariLifecycle::AikariSharedInstances::getInstance();
            auto* fsMgr = sharedIns.getPtr(
                &AikariTypes::Global::Lifecycle::SharedInstances::fsManagerIns
            );
            const std::string dirType = param.value("dirType", "");
            nlohmann::json result;
            result["success"] = true;
            if (dirType == "auraRoot")
            {
                result["path"] = fsMgr->hugoAuraRootDir.string();
            }
            else if (dirType == "aikariRoot")
            {
                result["path"] = fsMgr->aikariRootDir.string();
            }
            else if (dirType == "aikariConf")
            {
                result["path"] = fsMgr->aikariConfigDir.string();
            }
            else if (dirType == "aikariLog")
            {
                result["path"] = fsMgr->aikariLogDir.string();
            }
            else
            {
                result["diagnoseCode"] = itcConstants::Errors::INVALID_ARG;
                result["success"] = false;
            }

            return result;
        }
    }  // namespace Base

    itcTypes::MainToSubControlReplyMessage handleFsCtrlMessage(
        const itcTypes::SubToMainControlMessage& incoming,
        const std::vector<std::string>& routes
    )
    {
        const std::string& subRoute = routes.at(1);
        const std::string& fullRoute = incoming.method;
        itcTypes::MainToSubControlReplyMessage result = {
            .eventId = incoming.eventId
        };

        if (subRoute == itcConstants::FileSystem::Base::_PREFIX)
        {
            if (fullRoute == itcConstants::FileSystem::Base::GET_DIR)
            {
                result.data = Base::_getDir(incoming.data);
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
}  // namespace AikariLauncherRoutes::InterThread::FileSystem
