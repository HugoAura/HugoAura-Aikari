#pragma once

#include <Aikari-Launcher-Private/common.h>
#include <string>
#include <windows.h>

namespace AikariWinSvc
{
    namespace Messages
    {
        inline const std::string SCMgrOpenFailedMsg =
            "Failed to open sc manager, please check the permission status of "
            "Aikari.";
    }

    class WinSvcManager
    {
       public:
        bool isServiceExists;

        static bool checkSvcExists(const std::wstring& serviceNameW);

        WinSvcManager()
            : isServiceExists(
                  checkSvcExists(AikariDefaults::ServiceConfig::ServiceName)
              ) {};

        bool installService();

        bool uninstallService();
    };
}  // namespace AikariWinSvc
