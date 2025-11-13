#include "./basicRoute.h"

#include <Aikari-Launcher-Public/constants/ws/basic.h>
#include <Aikari-Launcher-Public/constants/ws/errorTemplates.h>
#include <Aikari-Launcher-Public/version.h>
#include <nlohmann/json.hpp>

namespace wsConstants = AikariLauncherPublic::Constants::WebSocket;

namespace AikariLauncherRoutes::WebSocket::Basic
{
    namespace Props
    {
        static AikariTypes::Components::WebSocket::ServerWSRep
        _impl_GET_VERSION()
        {
            return { .code = 0,
                     .success = true,
                     .data = {
                         { "version", AikariLauncherPublic::Version::Version },
                         { "versionCode",
                           AikariLauncherPublic::Version::VersionCode } } };
        }
    }  // namespace Props

    AikariTypes::Components::WebSocket::ServerWSRep handleBasicMethods(
        const AikariTypes::Components::WebSocket::ClientWSMsg&
            clientDataIncoming,
        const std::vector<std::string>& methods
    )
    {
        const std::string& subMethod = methods.at(1);
        const std::string& fullMethod = clientDataIncoming.method;
        AikariTypes::Components::WebSocket::ServerWSRep result;
        result.code =
            wsConstants::Errors::Codes::UNEXPECTED_ERROR;  // default val

        if (subMethod == wsConstants::Basic::Props::_PREFIX)
        {
            if (fullMethod == wsConstants::Basic::Props::GET_VERSION)
            {
                result = Props::_impl_GET_VERSION();
            }
            else
            {
                result = wsConstants::Errors::Templates::METHOD_NOT_FOUND;
            }
        }
        else
        {
            result = wsConstants::Errors::Templates::METHOD_NOT_FOUND;
        }

        return result;
    };
}  // namespace AikariLauncherRoutes::WebSocket::Basic
