#include "./basicRoute.h"

#include <Aikari-Launcher-Public/constants/ws/basic.h>
#include <Aikari-Launcher-Public/constants/ws/errorTemplates.h>
#include <Aikari-Shared/types/constants/version.h>
#include <nlohmann/json.hpp>

namespace wsConstants = AikariLauncher::Public::Constants::WebSocket;

namespace AikariLauncher::Routes::WebSocket::Basic
{
    namespace Props
    {
        static AikariLauncher::Public::Types::Components::WebSocket::ServerWSRep
        _impl_GET_VERSION()
        {
            return { .code = 0,
                     .success = true,
                     .data = {
                         { "version",
                           AikariShared::Constants::Version::Version },
                         { "versionCode",
                           AikariShared::Constants::Version::VersionCode } } };
        }
    }  // namespace Props

    AikariLauncher::Public::Types::Components::WebSocket::ServerWSRep
    handleBasicMethods(
        const AikariLauncher::Public::Types::Components::WebSocket::ClientWSMsg&
            clientDataIncoming,
        const std::vector<std::string>& methods
    )
    {
        const std::string& subMethod = methods.at(1);
        const std::string& fullMethod = clientDataIncoming.method;
        AikariLauncher::Public::Types::Components::WebSocket::ServerWSRep
            result;
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
}  // namespace AikariLauncher::Routes::WebSocket::Basic
