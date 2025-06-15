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
        : hugoAuraRootKey(L".DEFAULT\\SOFTWARE\\HugoAura"), aikariRootKey(hugoAuraRootKey + L"\\Aikari") {};

    int ensureRegKeyExists() const;
};
}  // namespace AikariRegistry
