#pragma once

#include "./windows/rc.hpp"
#include <string>

namespace AikariShared::utils::windows
{
    namespace network
    {
        typedef bool isSeewoCoreNeedToBeKill;
        isSeewoCoreNeedToBeKill ensureHostKeyExists(
            const std::string& hostLine
        );
    }  // namespace network

    namespace process
    {
        void killProcessByName(const std::string& procNameASCII);
    }

    namespace rc
    {
        // Implements in windows/rc.hpp
    }  // namespace rc
}  // namespace AikariShared::utils::windows
