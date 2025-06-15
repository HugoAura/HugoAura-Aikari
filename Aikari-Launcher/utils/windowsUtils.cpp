#include <Aikari-Launcher-Private/common.h>
#include <windows.h>

namespace AikariUtils::WindowsUtils
{
std::string WstringToString(const std::wstring& wstr)
{
    if (wstr.empty())
    {
        return std::string();
    }
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    if (sizeNeeded == 0)
    {
        return "WideCharToMultiByte failed to calculate size";
    }
    std::string strTo(sizeNeeded, 0);
    int result = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &strTo[0], sizeNeeded, NULL, NULL);

    if (result == 0)
    {
        return "WideCharToMultiByte failed to convert string";
    }
    return strTo;
}

std::string parseHResult(HRESULT hResult)
{
    LPSTR msgBuffer = NULL;
    size_t size =
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, hResult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuffer, 0, NULL);

    std::string finalStr(msgBuffer, size);
    LocalFree(msgBuffer);
    return finalStr;
}

std::string parseDWORDResult(DWORD result)
{
    LPWSTR msgBuffer = NULL;
    size_t size =
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msgBuffer, 0, NULL);
    if (size == 0)
    {
        return "Failed to exec FormatMessageW";
    }

    std::wstring messageW(msgBuffer, size);

    LocalFree(msgBuffer);

    std::string finMsg(WstringToString(messageW));
    return finMsg;
}
}  // namespace AikariUtils::WindowsUtils
