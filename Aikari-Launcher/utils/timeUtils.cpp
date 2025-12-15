#include "./timeUtils.h"

#include <date/tz.h>

namespace AikariUtils::TimeUtils
{
    void initTzDB(const std::filesystem::path& selfPath)
    {
        auto tzDbPath =
            selfPath.parent_path() / "resources" / "launcher" / "timezone";
        date::set_install(tzDbPath.string());
        date::reload_tzdb();
    }
}  // namespace AikariUtils::TimeUtils
