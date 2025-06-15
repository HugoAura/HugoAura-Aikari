#include "registry.h"

#include "../utils/windowsUtils.h"

namespace AikariRegistry
{
// --- Begin implementations of RegistryManager --- //
int RegistryManager::ensureRegKeyExists() const
{
    int success = 0;
    HKEY hKey;
    LOG_INFO("Checking aikari root regKey...");

    // LOG_DEBUG(std::format("Target key: {}", static_cast<char>(this->aikariRootKey)));

    LONG result = RegCreateKeyEx(HKEY_USERS, this->aikariRootKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE,
                                 KEY_ALL_ACCESS, NULL, &hKey, NULL);

    if (result == ERROR_SUCCESS)
    {
        LOG_INFO("Reg key confirmation succeed");
    }
    else
    {
        std::string parsedErr(AikariUtils::WindowsUtils::parseDWORDResult(result));
        LOG_CRITICAL("Unexpected error occurred creating aikari root regKey: " + parsedErr);
        success = -1;
    }

    RegCloseKey(hKey);
    return success;
};
// --- End implementations of RegistryManager --- //
}  // namespace AikariRegistry
