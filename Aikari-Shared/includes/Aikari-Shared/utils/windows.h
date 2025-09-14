#pragma once

#include <string>

#include "./windows/rc.hpp"

namespace AikariShared::Utils::Windows
{
    namespace Network
    {
        typedef bool isSeewoCoreNeedToBeKill;
        isSeewoCoreNeedToBeKill ensureHostKeyExists(
            const std::string& hostLine
        );
    }  // namespace Network

    namespace Process
    {
        void killProcessByName(const std::string& procNameASCII);
    }

    namespace RC
    {
        // Implements in windows/rc.hpp
    }  // namespace RC
}  // namespace AikariShared::Utils::Windows
