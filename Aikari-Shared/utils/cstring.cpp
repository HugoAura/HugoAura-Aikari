#include <Aikari-Shared/utils/cstring.h>
#include <ranges>
#include <string_view>

namespace AikariShared::Utils::CString
{
    std::string joinCStringArr(
        const char* const* charArr, const char* delim, int size
    )
    {
        size_t totalLength = 0;
        for (size_t i = 0; i < size; ++i)
        {
            if (charArr[i])
                totalLength += std::strlen(charArr[i]);
        }

        std::string result;
        result.reserve(totalLength);

        for (size_t i = 0; i < size; ++i)
        {
            if (charArr[i])
            {
                result.append(charArr[i]);
                result.append(delim);
            }
        }

        return result;
    };
}  // namespace AikariShared::Utils::CString
