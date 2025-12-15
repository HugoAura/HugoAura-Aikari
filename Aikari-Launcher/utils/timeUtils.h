#pragma once

#include <filesystem>

namespace AikariUtils::TimeUtils
{
    void initTzDB(const std::filesystem::path& selfPath);
}
