#pragma once

#include <Aikari-Launcher-Private/common.h>
#include <windows.h>

namespace AikariUtils::WindowsUtils
{
std::string WstringToString(const std::wstring& wstr);
std::wstring StringToWstring(const std::string& str);
std::string parseHResult(HRESULT hResult);
std::string parseDWORDResult(DWORD result);
}  // namespace AikariUtils::WindowsUtils
