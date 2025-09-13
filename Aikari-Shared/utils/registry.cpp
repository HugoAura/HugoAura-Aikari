#include <Aikari-Shared/utils/registry.h>  // self
#include <Aikari-Shared/utils/string.h>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>
#include <windows.h>

namespace winStrUtils = AikariShared::utils::string;

namespace AikariShared::utils::windowsRegistry
{
    std::wstring getRegSzValue(
        HKEY rootKey, const std::wstring& folderKey, const std::wstring& valKey
    )
    {
        HKEY hKeyIns;
        LONG result =
            RegOpenKeyExW(rootKey, folderKey.c_str(), 0, KEY_READ, &hKeyIns);
        if (result != ERROR_SUCCESS)
        {
            throw std::runtime_error(
                std::format(
                    "Failed to open target reg key, error: {}",
                    winStrUtils::parseDWORDResult(GetLastError())
                )
            );
        }

        DWORD dataType;
        DWORD dataSize = 0;

        result = RegQueryValueExW(
            hKeyIns, valKey.c_str(), NULL, &dataType, NULL, &dataSize
        );
        if (result != ERROR_SUCCESS)
        {
            RegCloseKey(hKeyIns);
            throw std::runtime_error(
                std::format(
                    "Failed to query target entry, error: {}",
                    winStrUtils::parseDWORDResult(GetLastError())
                )
            );
        }

        if (dataType != REG_SZ && dataType != REG_EXPAND_SZ)
        {
            RegCloseKey(hKeyIns);
            throw std::runtime_error(
                "Unsupported key type, only REG_*SZ is supported."
            );
        }

        std::vector<wchar_t> dataBuffer(dataSize / sizeof(wchar_t));
        result = RegQueryValueExW(
            hKeyIns,
            valKey.c_str(),
            NULL,
            NULL,
            reinterpret_cast<LPBYTE>(dataBuffer.data()),
            &dataSize
        );

        if (result != ERROR_SUCCESS)
        {
            RegCloseKey(hKeyIns);
            throw std::runtime_error(
                std::format(
                    "Failed to read value from reg entry, error: {}",
                    winStrUtils::parseDWORDResult(GetLastError())
                )
            );
        }

        if (!dataBuffer.empty() && dataBuffer.back() != L'\0')
        {
            dataBuffer.emplace_back(L'\0');
        }

        RegCloseKey(hKeyIns);

        std::wstring finalStr(dataBuffer.data());
        return finalStr;
    };
}  // namespace AikariShared::utils::windowsRegistry
