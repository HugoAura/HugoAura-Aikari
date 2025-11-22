#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <windows.h>

namespace AikariShared::Utils::FileSystem
{
    std::filesystem::path getSelfPathFromHandler(HMODULE hModule);

    void grepFilesWithExt(
        const std::filesystem::path& dir,
        const std::string& extToMatch,
        std::vector<std::filesystem::path>* resultTarget
    );

    std::string readFile(const std::filesystem::path& path);

    std::filesystem::path getProgramDataPath(bool showLog = false);
    std::filesystem::path getAikariRootDir(bool showLog = false);
}  // namespace AikariShared::Utils::FileSystem
