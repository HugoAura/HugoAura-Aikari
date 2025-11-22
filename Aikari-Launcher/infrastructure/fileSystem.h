#pragma once

#include <filesystem>

namespace AikariFileSystem
{
    class FileSystemManager
    {
       public:
        std::filesystem::path hugoAuraRootDir;
        std::filesystem::path aikariRootDir;
        std::filesystem::path aikariConfigDir;
        std::filesystem::path aikariLogDir;

        FileSystemManager();

        // [!IMPORTANT] If aikariLogDir / aikariRootDir changed,
        // /logger.cpp:105/ should be updated too

        bool ensureDirExists() const;
    };
};  // namespace AikariFileSystem
