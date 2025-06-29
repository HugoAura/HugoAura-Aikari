#pragma once

#include <string>
#include <wtypes.h>

namespace AikariShared::utils::string
{
std::string WstringToString(const std::wstring& wstr);
std::wstring StringToWstring(const std::string& str);
std::string parseHResult(HRESULT hResult);
std::string parseDWORDResult(DWORD result);
std::wstring expandEnvStr(const std::wstring& src);
}  // namespace AikariShared::utils::string
