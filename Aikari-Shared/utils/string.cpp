#include <Aikari-Shared/utils/string.h>  // self
#include <format>
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
};  // namespace AikariShared::Utils::String
