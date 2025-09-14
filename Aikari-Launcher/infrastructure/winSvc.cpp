#include "winSvc.h"

#include <Aikari-Shared/utils/string.h>
#include <windows.h>

namespace svcConfig = AikariDefaults::ServiceConfig;

namespace AikariWinSvc
{
    bool WinSvcManager::checkSvcExists(const std::wstring& serviceNameW)
    {
        SC_HANDLE hSCMgr = OpenSCManagerW(NULL, NULL, GENERIC_READ);
        if (!hSCMgr)
        {
            LOG_ERROR(AikariWinSvc::Messages::SCMgrOpenFailedMsg);
            return false;
        }

        SC_HANDLE hSvcIns =
            OpenServiceW(hSCMgr, serviceNameW.c_str(), SERVICE_QUERY_STATUS);

        bool isExists = false;

        if (hSvcIns != NULL)
        {
            isExists = true;
            CloseServiceHandle(hSvcIns);
        }
        else
        {
            DWORD lastErr = GetLastError();
            if (lastErr != ERROR_SERVICE_DOES_NOT_EXIST)
            {
                LOG_WARN(
                    "Failed to query service status, considering as not exists."
                );
            }
            isExists = false;
        }

        CloseServiceHandle(hSCMgr);
        return isExists;
    }

    bool WinSvcManager::installService()
    {
        wchar_t szPath[MAX_PATH];
        DWORD getPathResult = GetModuleFileNameW(NULL, szPath, MAX_PATH);

        if (getPathResult == 0)
        {
            LOG_ERROR(
                "Failed to get Aikari executable path. Canceling service "
                "installation."
            );
            return false;
        }

        std::wstring fullLaunchParam = L"\"";
        fullLaunchParam += szPath;
        fullLaunchParam += L"\"";
        fullLaunchParam += L" ";
        fullLaunchParam += svcConfig::StartArg;

        LOG_DEBUG(
            "Service launchParam to be installed: " +
            AikariShared::Utils::String::WstringToString(fullLaunchParam)
        );

        SC_HANDLE hSCMgr =
            OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        if (!hSCMgr)
        {
            LOG_ERROR(AikariWinSvc::Messages::SCMgrOpenFailedMsg);
            return false;
        }

        const TCHAR* svcDeps = TEXT("Tcpip\0RpcSs\0EventLog\0Dnscache\0");

        SC_HANDLE hSvcIns = CreateServiceW(
            hSCMgr,
            svcConfig::ServiceName.c_str(),
            svcConfig::ServiceDisplayName.c_str(),
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START,
            SERVICE_ERROR_NORMAL,
            fullLaunchParam.c_str(),
            NULL,
            NULL,
            reinterpret_cast<LPCWSTR>(svcDeps),
            NULL,
            NULL
        );

        bool isSuccess = false;
        if (hSvcIns)
        {
            LOG_INFO("Aikari Svc register success");
            SERVICE_DESCRIPTIONW svcDescIns;
            svcDescIns.lpDescription = (LPWSTR)svcConfig::ServiceDesc.c_str();

            ChangeServiceConfig2W(
                hSvcIns, SERVICE_CONFIG_DESCRIPTION, &svcDescIns
            );

            CloseServiceHandle(hSvcIns);
            isSuccess = true;
        }
        else
        {
            auto error = GetLastError();
            std::string resolvedErrMsg(
                AikariShared::Utils::String::parseDWORDResult(error)
            );
            LOG_ERROR(
                "Failed to register Aikari Svc, error: " + resolvedErrMsg
            );
        }

        CloseServiceHandle(hSCMgr);
        return isSuccess;
    }

    bool WinSvcManager::uninstallService()
    {
        LOG_INFO("Uninstalling Aikari Svc...");
        SC_HANDLE hSCMgr = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!hSCMgr)
        {
            LOG_ERROR(AikariWinSvc::Messages::SCMgrOpenFailedMsg);
            return false;
        }

        SC_HANDLE hSvcIns = OpenServiceW(
            hSCMgr, svcConfig::ServiceName.c_str(), SERVICE_ALL_ACCESS
        );
        if (!hSvcIns)
        {
            auto error = GetLastError();
            if (error == ERROR_SERVICE_DOES_NOT_EXIST)
            {
                LOG_WARN("Aikari Svc hasn't been installed.");
            }
            else
            {
                std::string parsedErr(
                    AikariShared::Utils::String::parseDWORDResult(error)
                );
                LOG_ERROR(
                    "Failed to open Aikari Svc instance, error: " + parsedErr
                );
            }
            CloseServiceHandle(hSCMgr);
            return false;
        }

        SERVICE_STATUS_PROCESS svcStatusProcess;
        DWORD bytesNeeded = 0;

        if (QueryServiceStatusEx(
                hSvcIns,
                SC_STATUS_PROCESS_INFO,
                (LPBYTE)&svcStatusProcess,
                sizeof(svcStatusProcess),
                &bytesNeeded
            ))
        {
            if (svcStatusProcess.dwCurrentState != SERVICE_STOPPED &&
                svcStatusProcess.dwCurrentState != SERVICE_STOP_PENDING)
            {
                LOG_INFO("Aikari Svc is running, stopping it...");
                if (ControlService(
                        hSvcIns,
                        SERVICE_CONTROL_STOP,
                        (LPSERVICE_STATUS)&svcStatusProcess
                    ))
                {
                    LOG_INFO("Aikari Svc stopped");
                }
                else
                {
                    LOG_ERROR(
                        "Unexpected error occurred while stopping svc, proceed "
                        "to "
                        "force delete."
                    );
                }
            }
        }

        bool isSuccess = false;

        if (DeleteService(hSvcIns))
        {
            LOG_INFO("Aikari Svc uninstalled successfully");
            isSuccess = true;
        }
        else
        {
            auto error = GetLastError();
            std::string parsedErr(
                AikariShared::Utils::String::parseDWORDResult(error)
            );
            LOG_ERROR(
                "An error occurred uninstalling svc, error: " + parsedErr
            );
        }

        CloseServiceHandle(hSvcIns);
        CloseServiceHandle(hSCMgr);
        return isSuccess;
    }
}  // namespace AikariWinSvc
