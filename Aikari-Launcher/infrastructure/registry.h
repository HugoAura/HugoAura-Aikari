#pragma once

#include <Aikari-Launcher-Private/common.h>
#include <windows.h>

namespace AikariRegistry
{
    class RegistryManager
    {
       public:
        std::wstring hugoAuraRootKey;
        std::wstring aikariRootKey;

        RegistryManager()
            : hugoAuraRootKey(L".DEFAULT\\SOFTWARE\\HugoAura"),
              aikariRootKey(hugoAuraRootKey + L"\\Aikari") {};

        int ensureRegKeyExists() const;

        int writeRegKeySz(
            std::wstring key, std::wstring entry, std::wstring data
        );

        int writeRegKeySz(
            std::wstring key,
            std::vector<std::wstring> entry,
            std::vector<std::wstring> data
        );

       private:
        bool checkResult(LONG errorCode, const std::string& action);
    };
}  // namespace AikariRegistry
