#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <windows.h>

namespace AikariShared::utils::filesystem
{
    std::filesystem::path getSelfPathFromHandler(HMODULE hModule);

    void grepFilesWithExt(
        const std::filesystem::path& dir,
        const std::string& extToMatch,
        std::vector<std::filesystem::path>* resultTarget
    );
}  // namespace AikariShared::utils::filesystem
