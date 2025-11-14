#pragma once

#include <string>
#include <windows.h>

namespace AikariShared::Utils::String
{
    std::vector<std::string> split(const std::string& s, char delim);
    std::string replaceAll(
        std::string_view str, std::string_view from, std::string_view to
    );
}  // namespace AikariShared::Utils::String
