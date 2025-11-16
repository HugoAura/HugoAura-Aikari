#pragma once

#include <string>
#include <windows.h>

#include "./virtual/lifecycle/ILaunchProgressReporter.h"

namespace AikariLauncher::Core::WinSCM
{
    static DWORD dwControlsAccepted =
        SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    constexpr DWORD initialWaitHint = 2000;

    class WinSvcLaunchProgressReporter
        : public AikariLauncher::Virtual::Lifecycle::IProgressReporter
    {
       public:
        explicit WinSvcLaunchProgressReporter(
            std::function<void(
                DWORD dwordCurState,
                DWORD dwordWin32ExitCode,
                DWORD dwordWaitHint
            )>& caller
        );

        std::function<void(
            DWORD dwordCurState, DWORD dwordWin32ExitCode, DWORD dwordWaitHint
        )>
            reportFn;

        void reportLaunchProgress(
            bool isFailed,
            bool isCompleted,
            bool isStarting,
            unsigned int waitForMs,
            unsigned int percent,
            unsigned int exitCode
        ) override final;
    };

    class IWinSvcHandler
    {
       public:
        explicit IWinSvcHandler(const std::wstring& serviceName);
        virtual ~IWinSvcHandler() = default;

        IWinSvcHandler(const IWinSvcHandler&) = delete;
        IWinSvcHandler& operator=(const IWinSvcHandler&) = delete;

        bool startAsService();

       protected:
        std::wstring serviceName;
        SERVICE_STATUS_HANDLE hServiceStatus;
        SERVICE_STATUS curServiceStatus;
        HANDLE hServiceStopEvent;

        DWORD statusUpdateCheckpoint;

        virtual void onServiceStart(DWORD dwordArgc, LPWSTR* lpszArgv) = 0;
        virtual void onServiceStop();

        virtual void reportServiceStatus(
            DWORD dwordCurState, DWORD dwordWin32ExitCode, DWORD dwordWaitHint
        );

       private:
        static IWinSvcHandler* selfInsPtr;

        static VOID WINAPI jmpServiceMain(DWORD dwordArgc, LPWSTR* lpszArgv);
        static DWORD WINAPI jmpServiceCtrlHandler(
            DWORD dwordCtrl, DWORD dwordEvtType, LPVOID lpEvtData, LPVOID lpCtx
        );
        void cxxInternalServiceMain(DWORD dwordArgc, LPWSTR* lpszArgv);
        DWORD cxxInternalServiceCtrlHandler(DWORD dwordCtrl);
    };
}  // namespace AikariLauncher::Core::WinSCM
