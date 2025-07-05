#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/utils/registry.h>
#include <Aikari-Shared/utils/string.h>
#include <Aikari-Shared/utils/windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <windows.h>

namespace registryUtils = AikariShared::utils::windowsRegistry;
namespace stringUtils = AikariShared::utils::string;

namespace AikariShared::utils::windows::network
{
    // ↓ some constants
    const wchar_t* networkDBRegKeyPath =
        L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters";

    const wchar_t* networkDBRegKeyEntryName = L"DataBasePath";

    const wchar_t* defaultHostPath = L"%SystemRoot%\\System32\\drivers\\etc";
    const wchar_t* defaultHostPathExpanded =
        L"C:\\Windows\\System32\\drivers\\etc";
    // ↑ end of constants

    isSeewoCoreNeedToBeKill ensureHostKeyExists(const std::string& hostLine)
    {
        const int SW_CORE_DONT_NEED_KILL = false;
        const int SW_CORE_NEED_KILL = true;

        std::wstring hostDir;
        try
        {
            hostDir = registryUtils::getRegSzValue(
                HKEY_LOCAL_MACHINE,
                std::wstring(networkDBRegKeyPath),
                std::wstring(networkDBRegKeyEntryName)
            );
        }
        catch (const std::exception& err)
        {
            LOG_ERROR(
                "Unexpected error occurred querying hostPath: {}", err.what()
            );
            LOG_ERROR("Assuming to use the default path.");
            hostDir = std::wstring(defaultHostPath);
        }

        // ↑ hostPath with env var
        // ↓ resolve hostPath

        try
        {
            hostDir = stringUtils::expandEnvStr(hostDir);
            LOG_DEBUG(
                "Successfully parsed host path, result: {}",
                stringUtils::WstringToString(hostDir)
            );
        }
        catch (const std::exception& err)
        {
            LOG_ERROR(
                "Unexpected error occurred expanding env vars in hostPath: {}",
                err.what()
            );
            LOG_ERROR("Assuming to use the default expanded path.");
            hostDir = std::wstring(defaultHostPathExpanded);
        }

        // ↑ hostPath parsed

        std::filesystem::path hostDirIns(stringUtils::WstringToString(hostDir));
        std::filesystem::path hostPathIns = hostDirIns / "hosts";

        if (!std::filesystem::exists(hostPathIns))
        {
            LOG_ERROR("Failed to write host: hosts file not found.");
            return SW_CORE_DONT_NEED_KILL;
        }

        isSeewoCoreNeedToBeKill result = SW_CORE_NEED_KILL;

        {
            std::ifstream hostsFile(hostPathIns);

            if (!hostsFile)
            {
                LOG_ERROR("Failed to write host: hosts file failed to open.");
                return SW_CORE_DONT_NEED_KILL;
            }

            std::string perLine;

            while (std::getline(hostsFile, perLine))
            {
                if (perLine.find(hostLine) != std::string::npos)
                {
                    LOG_INFO(
                        "Found target line in hosts file, no need to kill "
                        "swCore."
                    );
                    result = SW_CORE_DONT_NEED_KILL;
                    break;
                }
            };

            if (result == SW_CORE_DONT_NEED_KILL)
            {
                hostsFile.close();
                return result;
            }

            // not found
            hostsFile.close();

            std::ofstream hostsFileWrite(hostPathIns, std::ios::app);
            if (!hostsFileWrite)
            {
                LOG_ERROR(
                    "Failed to get write access to hosts file, r u running "
                    "Aikari "
                    "with Administrator privilege?"
                );
                result = SW_CORE_DONT_NEED_KILL;
                return result;
            }

            hostsFileWrite << "\n" + hostLine;

            hostsFileWrite.close();

            LOG_INFO("Successfully appended `{}` into hosts file;", hostLine);
        }

        return result;
    }
}  // namespace AikariShared::utils::windows::network
