#include "./winSvcHandler.h"

#define CUSTOM_LOG_HEADER "[WinSvc Controller]"

#include <Aikari-Shared/infrastructure/loggerMacro.h>

namespace AikariLauncher::Core::WinSCM
{
    // ↓ Begin implementation of WinSvcLaunchProgressReporter ↓ //
    WinSvcLaunchProgressReporter::WinSvcLaunchProgressReporter(
        std::function<void(
            DWORD dwordCurState, DWORD dwordWin32ExitCode, DWORD dwordWaitHint
        )>& caller
    )
        : reportFn(std::move(caller)) {};

    void WinSvcLaunchProgressReporter::reportLaunchProgress(
        bool isFailed,
        bool isCompleted,
        bool isStarting,
        unsigned int waitForMs,
        unsigned int percent,
        unsigned int exitCode
    )
    {
#ifdef _DEBUG
        CUSTOM_LOG_INFO(
            "Updated launch progress: "
            "[isFailed={},isCompleted={},isStarting={},waitForMs={}]",
            isFailed,
            isCompleted,
            isStarting,
            waitForMs
        );
#endif
        DWORD nextState;
        DWORD waitHint;
        if (isFailed)
        {
            nextState = SERVICE_STOPPED;
            waitHint = 0;
        }
        else
        {
            if (isCompleted)
            {
                nextState = isStarting ? SERVICE_RUNNING : SERVICE_STOPPED;
            }
            else
            {
                nextState =
                    isStarting ? SERVICE_START_PENDING : SERVICE_STOP_PENDING;
            }
            waitHint = static_cast<DWORD>(waitForMs);
        }
        this->reportFn(nextState, exitCode, waitHint);
    };
    // ↑ End implementation of WinSvcLaunchProgressReporter ↑ //

    // ↓ Begin implementation of IWinSvcHandler ↓ //
    AikariLauncher::Core::WinSCM::IWinSvcHandler*
        AikariLauncher::Core::WinSCM::IWinSvcHandler::selfInsPtr = nullptr;

    IWinSvcHandler::IWinSvcHandler(const std::wstring& serviceName)
        : serviceName(serviceName)
    {
        this->curServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        this->curServiceStatus.dwCurrentState = SERVICE_STOPPED;
        this->curServiceStatus.dwControlsAccepted = 0;
        this->curServiceStatus.dwWin32ExitCode = NO_ERROR;
        this->curServiceStatus.dwCheckPoint = 0;
        this->curServiceStatus.dwWaitHint = 0;
        this->curServiceStatus.dwServiceSpecificExitCode = 0;

        this->statusUpdateCheckpoint = 0;
    };

    bool IWinSvcHandler::startAsService()
    {
        IWinSvcHandler::selfInsPtr = this;

        const SERVICE_TABLE_ENTRYW srvDispatchTable[] = {
            { (LPWSTR)this->serviceName.c_str(),
              (LPSERVICE_MAIN_FUNCTIONW)IWinSvcHandler::jmpServiceMain },
            { NULL, NULL }
        };

        return StartServiceCtrlDispatcherW(srvDispatchTable);
    };

    void IWinSvcHandler::onServiceStop()
    {
        if (this->hServiceStopEvent)
        {
            SetEvent(this->hServiceStopEvent);
        }
    };

    void IWinSvcHandler::reportServiceStatus(
        DWORD dwordCurState, DWORD dwordWin32ExitCode, DWORD dwordWaitHint
    )
    {
        this->curServiceStatus.dwCurrentState = dwordCurState;
        this->curServiceStatus.dwWin32ExitCode = dwordWin32ExitCode;
        this->curServiceStatus.dwWaitHint = dwordWaitHint;

        if (dwordCurState == SERVICE_START_PENDING ||
            dwordCurState == SERVICE_STOP_PENDING ||
            dwordCurState == SERVICE_CONTINUE_PENDING ||
            dwordCurState ==
                SERVICE_PAUSE_PENDING)  // 其实 continue 和 pause 没必要写
        {
            this->statusUpdateCheckpoint++;
        }
        else
        {
            this->statusUpdateCheckpoint = 0;
        }
        this->curServiceStatus.dwCheckPoint = this->statusUpdateCheckpoint;

        if (dwordCurState == SERVICE_RUNNING || dwordCurState == SERVICE_PAUSED)
        {
            this->curServiceStatus.dwControlsAccepted =
                AikariLauncher::Core::WinSCM::dwControlsAccepted;
        }
        else
        {
            this->curServiceStatus.dwControlsAccepted = 0;
        }

        SetServiceStatus(this->hServiceStatus, &this->curServiceStatus);
    };

    VOID WINAPI
    IWinSvcHandler::jmpServiceMain(DWORD dwordArgc, LPWSTR* lpszArgv)
    {
        if (IWinSvcHandler::selfInsPtr)
        {
            IWinSvcHandler::selfInsPtr->cxxInternalServiceMain(
                dwordArgc, lpszArgv
            );
        }
    };

    DWORD WINAPI IWinSvcHandler::jmpServiceCtrlHandler(
        DWORD dwordCtrl, DWORD dwordEvtType, LPVOID lpEvtData, LPVOID lpCtx
    )
    {
        if (IWinSvcHandler::selfInsPtr)
        {
            return IWinSvcHandler::selfInsPtr->cxxInternalServiceCtrlHandler(
                dwordCtrl
            );
        }
        return NO_ERROR;
    };

    void IWinSvcHandler::cxxInternalServiceMain(
        DWORD dwordArgc, LPWSTR* lpszArgv
    )
    {
        this->hServiceStatus = RegisterServiceCtrlHandlerExW(
            this->serviceName.c_str(),
            IWinSvcHandler::jmpServiceCtrlHandler,
            NULL
        );
        if (!this->hServiceStatus)
        {
            // TODO: Error handling
            return;
        }
        this->hServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (this->hServiceStopEvent == NULL)
        {
            this->reportServiceStatus(SERVICE_STOPPED, GetLastError(), 0);
            return;
        }
        this->reportServiceStatus(
            SERVICE_START_PENDING,
            NO_ERROR,
            AikariLauncher::Core::WinSCM::initialWaitHint
        );

        this->onServiceStart(dwordArgc, lpszArgv);

        CloseHandle(this->hServiceStopEvent);
        this->hServiceStopEvent = NULL;

        this->reportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    };

    DWORD IWinSvcHandler::cxxInternalServiceCtrlHandler(DWORD dwordCtrl)
    {
        switch (dwordCtrl)
        {
            case SERVICE_CONTROL_PAUSE:
            case SERVICE_CONTROL_CONTINUE:
                return ERROR_CALL_NOT_IMPLEMENTED;
            case SERVICE_CONTROL_STOP:
            case SERVICE_CONTROL_SHUTDOWN:
            {
                this->reportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
                this->onServiceStop();
                return NO_ERROR;
            }
            case SERVICE_CONTROL_INTERROGATE:
                return NO_ERROR;
            case SERVICE_CONTROL_PARAMCHANGE:
            case SERVICE_CONTROL_PRESHUTDOWN:
                return NO_ERROR;
            default:
                return NO_ERROR;
        }
    };
    // ↑ End implementation of IWinSvcHandler ↑ //
}  // namespace AikariLauncher::Core::WinSCM
