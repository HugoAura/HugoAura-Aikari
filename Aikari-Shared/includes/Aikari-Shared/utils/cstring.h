#pragma once

#include <string>

namespace AikariShared::Utils::CString
{
    std::string joinCStringArr(
        const char* const* charArr, const char* delim, int size
    );
}
