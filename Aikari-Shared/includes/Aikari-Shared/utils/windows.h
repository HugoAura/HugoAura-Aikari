#pragma once

#include <string>

namespace AikariShared::utils::windows
{
namespace network
{
typedef bool isSeewoCoreNeedToBeKill;
isSeewoCoreNeedToBeKill ensureHostKeyExists(const std::string& hostLine);
}  // namespace network

namespace process
{
void killProcessByName(const std::string& procNameASCII);
}
}  // namespace AikariShared::utils::windows
