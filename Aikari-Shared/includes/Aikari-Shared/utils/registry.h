#pragma once

#include <string>
#include <windows.h>

namespace AikariShared::utils::windowsRegistry
{
    std::wstring getRegSzValue(
        HKEY rootKey, const std::wstring& folderKey, const std::wstring& valKey
    );
}
