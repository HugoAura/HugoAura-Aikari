#include "./winSvc.h"

#define CUSTOM_LOG_HEADER "[Win Svc Manager]"

#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/utils/windows/winString.h>
#include <windows.h>

namespace svcConfig = AikariDefaults::ServiceConfig;

namespace AikariWinSvc
{
    bool WinSvcManager::checkSvcExists(const std::wstring& serviceNameW)
    {
        SC_HANDLE hSCMgr = OpenSCManagerW(NULL, NULL, GENERIC_READ);
        if (!hSCMgr)
        {
            CUSTOM_LOG_ERROR(
                "Failed to open SCM: {}",
                AikariWinSvc::Messages::SCMgrOpenFailedMsg
            );
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
                CUSTOM_LOG_WARN(
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
            CUSTOM_LOG_ERROR(
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

        CUSTOM_LOG_DEBUG(
            "Service launchParam to be installed: {}",
            AikariShared::Utils::Windows::WinString::WstringToString(
                fullLaunchParam
            )
        );

        SC_HANDLE hSCMgr =
            OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        if (!hSCMgr)
        {
            CUSTOM_LOG_ERROR(
                "Failed to open SCM: {}",
                AikariWinSvc::Messages::SCMgrOpenFailedMsg
            );
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
        ); /* 现在 Aikari 使用 Local System 身份启动, 这非常非常地危险
            * 在后续版本更新中, 高权限操作将被分离到独立的 Aikari-PAS 中
            * Aikari-Launcher 本身以用户身份或 Local Service 身份启动
            * (具体启动 & 鉴权方式待定)
            */

        bool isSuccess = false;
        if (hSvcIns)
        {
            CUSTOM_LOG_INFO("Aikari Svc register success");
            isSuccess = true;

            try
            {
                SERVICE_DESCRIPTIONW svcDescIns;
                svcDescIns.lpDescription =
                    (LPWSTR)svcConfig::ServiceDesc.c_str();
                ChangeServiceConfig2W(
                    hSvcIns, SERVICE_CONFIG_DESCRIPTION, &svcDescIns
                );

                SC_ACTION svcFailureActions[4];
                svcFailureActions[0].Type = SC_ACTION_RESTART;
                svcFailureActions[0].Delay = 1000;
                svcFailureActions[1].Type = SC_ACTION_RESTART;
                svcFailureActions[1].Delay = 3000;
                svcFailureActions[2].Type = SC_ACTION_RESTART;
                svcFailureActions[2].Delay = 2000;
                svcFailureActions[3].Type = SC_ACTION_NONE;
                svcFailureActions[3].Delay = 0;

                SERVICE_FAILURE_ACTIONS svcFailureActionConfig;
                svcFailureActionConfig.dwResetPeriod = 120;
                svcFailureActionConfig.cActions = 4;
                svcFailureActionConfig.lpRebootMsg = nullptr;
                svcFailureActionConfig.lpCommand = nullptr;
                svcFailureActionConfig.lpsaActions = svcFailureActions;
                if (!ChangeServiceConfig2W(
                        hSvcIns,
                        SERVICE_CONFIG_FAILURE_ACTIONS,
                        &svcFailureActionConfig
                    ))
                {
                    auto err = GetLastError();
                    std::string resolvedErrMsg(
                        AikariShared::Utils::Windows::WinString::
                            parseDWORDResult(err)
                    );
                    throw std::runtime_error(
                        std::format(
                            "Error setting failure actions for Aikari Svc, "
                            "error: "
                            "{}",
                            resolvedErrMsg
                        )
                    );
                }

                CloseServiceHandle(hSvcIns);
            }
            catch (const std::exception& e)
            {
                CloseServiceHandle(hSvcIns);
                CUSTOM_LOG_ERROR(
                    "Unexpected error occurred during the post-setup stage of "
                    "installing Aikari Svc, error: {}",
                    e.what()
                );
            }
        }
        else
        {
            auto error = GetLastError();
            std::string resolvedErrMsg(
                AikariShared::Utils::Windows::WinString::parseDWORDResult(error)
            );
            CUSTOM_LOG_ERROR(
                "Failed to register Aikari Svc, error: {}", resolvedErrMsg
            );
        }

        CloseServiceHandle(hSCMgr);
        return isSuccess;
    }

    bool WinSvcManager::uninstallService()
    {
        CUSTOM_LOG_INFO("Uninstalling Aikari Svc...");
        SC_HANDLE hSCMgr = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!hSCMgr)
        {
            CUSTOM_LOG_ERROR(
                "Failed to open SCM: {}",
                AikariWinSvc::Messages::SCMgrOpenFailedMsg
            );
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
                CUSTOM_LOG_WARN("Aikari Svc hasn't been installed.");
            }
            else
            {
                std::string parsedErr(
                    AikariShared::Utils::Windows::WinString::parseDWORDResult(
                        error
                    )
                );
                CUSTOM_LOG_ERROR(
                    "Failed to open Aikari Svc instance, error: {}", parsedErr
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
                CUSTOM_LOG_INFO("Aikari Svc is running, stopping it...");
                if (ControlService(
                        hSvcIns,
                        SERVICE_CONTROL_STOP,
                        (LPSERVICE_STATUS)&svcStatusProcess
                    ))
                {
                    CUSTOM_LOG_INFO("Aikari Svc stopped");
                }
                else
                {
                    CUSTOM_LOG_ERROR(
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
            CUSTOM_LOG_INFO("Aikari Svc uninstalled successfully");
            isSuccess = true;
        }
        else
        {
            auto error = GetLastError();
            std::string parsedErr(
                AikariShared::Utils::Windows::WinString::parseDWORDResult(error)
            );
            CUSTOM_LOG_ERROR(
                "An error occurred uninstalling svc, error: {}", parsedErr
            );
        }

        CloseServiceHandle(hSvcIns);
        CloseServiceHandle(hSCMgr);
        return isSuccess;
    }
}  // namespace AikariWinSvc
