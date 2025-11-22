#include <Aikari-Shared/utils/string.h>  // self
#include <format>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace AikariShared::Utils::String
{
    std::vector<std::string> split(const std::string& s, char delim)
    {
        std::stringstream ss(s);
        std::string item;
        std::vector<std::string> elems;
        while (std::getline(ss, item, delim))
        {
            elems.push_back(std::move(item));
        }
        return elems;
    }

    std::string replaceAll(
        std::string_view str, std::string_view from, std::string_view to
    )
    {
        auto parts = str | std::views::split(from);
        auto joined = parts | std::views::join_with(to) | std::views::common;
        std::string result = std::ranges::to<std::string>(joined);
        return result;
    }

    std::string trim(const std::string& str)
    {
        std::string result;
        std::string::size_type startPos = str.find_first_not_of(' ');
        if (startPos == std::string::npos)
        {
            return result;
        }
        std::string::size_type endPos = str.find_last_not_of(' ');
        if (endPos != std::string::npos)
        {
            result = str.substr(startPos, endPos - startPos + 1);
        }
        else
        {
            result = str.substr(startPos);
        }

        return result;
    };
};  // namespace AikariShared::Utils::String
