#pragma once

#include <filesystem>
#include <windows.h>

namespace AikariShared::utils::filesystem
{
    std::filesystem::path getSelfPathFromHandler(HMODULE hModule);
}  // namespace AikariShared::utils::filesystem
