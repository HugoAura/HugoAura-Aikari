#include "registry.h"

#include <Aikari-Shared/utils/string.h>

namespace AikariRegistry
{
    // --- Begin implementations of RegistryManager --- //
    int RegistryManager::ensureRegKeyExists() const
    {
        int success = 0;
        HKEY hKey;
        LOG_INFO("Checking aikari root regKey...");

        // LOG_DEBUG(std::format("Target key: {}",
        // static_cast<char>(this->aikariRootKey)));

        LONG result = RegCreateKeyEx(
            HKEY_USERS,
            this->aikariRootKey.c_str(),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            NULL
        );

        if (result == ERROR_SUCCESS)
        {
            LOG_INFO("Reg key confirmation succeed");
        }
        else
        {
            std::string parsedErr(
                AikariShared::utils::string::parseDWORDResult(result)
            );
            LOG_CRITICAL(
                "Unexpected error occurred creating aikari root regKey: " +
                parsedErr
            );
            success = -1;
        }

        RegCloseKey(hKey);
        return success;
    };

    int RegistryManager::writeRegKeySz(
        std::wstring key, std::wstring entry, std::wstring data
    )
    {
        int success = 0;
        HKEY hKey;

        LONG result = RegCreateKeyEx(
            HKEY_USERS,
            key.c_str(),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            NULL,
            &hKey,
            NULL
        );

        if (!this->checkResult(result, "getting reg key"))
        {
            success = -1;
        }
        else
        {
            result = RegSetValueExW(
                hKey,
                entry.c_str(),
                0,
                REG_SZ,
                (const BYTE*)data.c_str(),
                (wcslen(data.c_str()) + 1) * sizeof(wchar_t)
            );
            if (!this->checkResult(result, "writing key value"))
            {
                success = -1;
            }
        }

        RegCloseKey(hKey);
        return success;
    };

    int RegistryManager::writeRegKeySz(
        std::wstring key,
        std::vector<std::wstring> entries,
        std::vector<std::wstring> datas
    )
    {
        int success = 0;
        HKEY hKey;

        LONG result = RegCreateKeyExW(
            HKEY_USERS,
            key.c_str(),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            NULL,
            reinterpret_cast<PHKEY>(&hKey),
            NULL
        );

        if (!this->checkResult(result, "getting reg key"))
        {
            success = -1;
        }
        else
        {
            int itIdx = 0;
            for (const std::wstring& perEntry : entries)
            {
                std::wstring& data = datas.at(itIdx);
                result = RegSetValueExW(
                    hKey,
                    perEntry.c_str(),
                    0,
                    REG_SZ,
                    (const BYTE*)data.c_str(),
                    (wcslen(data.c_str()) + 1) * sizeof(wchar_t)
                );
                if (!this->checkResult(
                        result,
                        std::format("writing key value for idx: {}", itIdx)
                    ))
                {
                    success = -1;
                }

                itIdx++;
            }
        }

        RegCloseKey(hKey);
        return success;
    };

    bool RegistryManager::checkResult(LONG errorCode, const std::string& action)
    {
        if (errorCode != ERROR_SUCCESS)
        {
            LOG_ERROR(
                "An error occurred while {}, error code: {}",
                action,
                AikariShared::utils::string::parseDWORDResult(errorCode)
            );
            return false;
        }
        else
        {
            LOG_TRACE("Action: {} | Result: Success", action);
            return true;
        }
    };
    // --- End implementations of RegistryManager --- //
}  // namespace AikariRegistry
