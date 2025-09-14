#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/utils/windows.h>  // self
#include <TlHelp32.h>
#include <windows.h>

namespace AikariShared::Utils::Windows
{
    namespace Process
    {
        void killProcessByName(const std::string& procNameASCII)
        {
            std::wstring procNameWStr;

            try
            {
                procNameWStr = std::wstring(
                    procNameASCII.begin(), procNameASCII.end()
                );  // 没必要再去调那什么 StringToWString 了, 进程名一般不会是非
                    // ASCII 字符 (至少这个场景里)
            }
            catch (...)
            {
                throw std::runtime_error(
                    "Invalid procName provided, only ASCII process name "
                    "accepted."
                );
            }

            HANDLE hProcSnapshot = CreateToolhelp32Snapshot(
                TH32CS_SNAPPROCESS, 0
            );  // 2nd arg no meaning when snapProc
            if (hProcSnapshot == INVALID_HANDLE_VALUE)
            {
                throw std::runtime_error(
                    "Failed to open hProcSnapshot handle."
                );
            }

            PROCESSENTRY32W procEntry32;
            procEntry32.dwSize = sizeof(PROCESSENTRY32W);

            BOOL result = Process32FirstW(hProcSnapshot, &procEntry32);
            if (!result)
            {
                CloseHandle(hProcSnapshot);
                throw std::runtime_error("Failed to inspect the 1st process.");
            }

            LOG_INFO(
                "Searching for desired process to kill: {}", procNameASCII
            );

            do
            {
                if (wcscmp(procEntry32.szExeFile, procNameWStr.c_str()) == 0)
                {
                    LOG_INFO(
                        "Process {} with PID {} found.",
                        procNameASCII,
                        procEntry32.th32ProcessID
                    );

                    HANDLE hProcHandle = OpenProcess(
                        PROCESS_TERMINATE, FALSE, procEntry32.th32ProcessID
                    );

                    if (hProcHandle == NULL)
                    {
                        CloseHandle(hProcSnapshot);
                        throw std::runtime_error(
                            std::format(
                                "Failed to open process handle, PID: {}.",
                                procEntry32.th32ProcessID
                            )
                        );
                    }

                    result = TerminateProcess(hProcHandle, 0);
                    if (!result)
                    {
                        LOG_WARN(
                            "Failed to terminate process {} with PID {}",
                            procNameASCII,
                            procEntry32.th32ProcessID
                        );
                    }
                    else
                    {
                        LOG_INFO(
                            "Successfully terminated process {} with PID {}",
                            procNameASCII,
                            procEntry32.th32ProcessID
                        );
                    }

                    CloseHandle(hProcHandle);
                }
            } while (Process32NextW(hProcSnapshot, &procEntry32));

            CloseHandle(hProcSnapshot);
            LOG_DEBUG("KillProcByName task finished.");
        };
    }  // namespace process
}  // namespace AikariShared::utils::windows
