#pragma once

#include <string>
#include <windows.h>

namespace AikariShared::Utils::WindowsRegistry
{
    std::wstring getRegSzValue(
        HKEY rootKey, const std::wstring& folderKey, const std::wstring& valKey
    );
}
