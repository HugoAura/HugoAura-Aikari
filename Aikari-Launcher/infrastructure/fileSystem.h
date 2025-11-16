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

        static std::filesystem::path getProgramDataPath();

        FileSystemManager()
            : hugoAuraRootDir(
                  FileSystemManager::getProgramDataPath() / "HugoAura"
              ),
              aikariRootDir(hugoAuraRootDir / "Aikari"),
              aikariConfigDir(aikariRootDir / "config"),
              aikariLogDir(aikariRootDir / "log") {};

        // [!IMPORTANT] If aikariLogDir / aikariRootDir changed,
        // /logger.cpp:105/ should be updated too

        bool ensureDirExists() const;
    };
};  // namespace AikariFileSystem
