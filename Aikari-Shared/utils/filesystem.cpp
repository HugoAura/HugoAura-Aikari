#include <Aikari-Shared/utils/filesystem.h>  // self
#include <format>
#include <stdexcept>
#include <string>

namespace AikariShared::utils::filesystem
{
    std::filesystem::path getSelfPathFromHandler(HMODULE hModule)
    {
        wchar_t path[MAX_PATH];

        DWORD result = GetModuleFileNameW(hModule, path, MAX_PATH);
        if (result == 0)
        {
            DWORD errorCode = GetLastError();
            throw std::runtime_error(
                std::format(
                    "Error getting selfPath from HMODULE, errorCode: {}",
                    std::to_string(errorCode)
                )
            );
        }
        else
        {
            std::filesystem::path pathObj(path);
            return pathObj;
        }
    };
}  // namespace AikariShared::utils::filesystem