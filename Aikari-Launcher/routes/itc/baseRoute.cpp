#include "baseRoute.h"

#include <Aikari-Launcher-Public/constants/itcCtrl/base.h>
#include <Aikari-Launcher-Public/constants/itcCtrl/errorTemplates.h>
#include <nlohmann/json.hpp>

#include "../lifecycle.h"

namespace itcTypes = AikariShared::Types::InterThread;
namespace itcConstants = AikariLauncherPublic::Constants::InterThread;

namespace AikariLauncherRoutes::InterThread::Base
{
    namespace Props
    {
        static nlohmann::json _getRuntimeMode()
        {
            auto& lifecycleStates =
                AikariLifecycle::AikariStatesManager::getInstance();
            const auto runtimeMode =
                lifecycleStates.getVal(&AikariTypes::Global::Lifecycle::
                                           GlobalLifecycleStates::runtimeMode);

            nlohmann::json result;

            result["mode"] = static_cast<int>(runtimeMode);

            return result;
        };
    }  // namespace Props

    AikariShared::Types::InterThread::MainToSubControlReplyMessage
    handleBaseCtrlMsg(
        const AikariShared::Types::InterThread::SubToMainControlMessage&
            incoming,
        const std::vector<std::string>& routes
    )
    {
        const std::string& subRoute = routes.at(1);
        const std::string& fullRoute = incoming.method;
        itcTypes::MainToSubControlReplyMessage result = {
            .eventId = incoming.eventId
        };

        if (subRoute == itcConstants::Base::Props::_PREFIX)
        {
            if (fullRoute == itcConstants::Base::Props::GET_RUNTIME_MODE)
            {
                result.data = Props::_getRuntimeMode();
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
    }

}  // namespace AikariLauncherRoutes::InterThread::Base
