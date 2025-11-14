#pragma once

#include <string>
#include <windows.h>

namespace AikariShared::Utils::Windows::WinString
{
    std::string WstringToString(const std::wstring& wstr);
    std::wstring StringToWstring(const std::string& str);
    std::string parseHResult(HRESULT hResult);
    std::string parseDWORDResult(DWORD result);
    std::wstring expandEnvStr(const std::wstring& src);
}  // namespace AikariShared::Utils::Windows::WinString
