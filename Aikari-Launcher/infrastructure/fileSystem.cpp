#include "fileSystem.h"

#define CUSTOM_LOG_HEADER "[FS Manager]"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/utils/string.h>
#include <Aikari-Shared/utils/windows/winString.h>
#include <ShlObj.h>
#include <filesystem>

#include "Aikari-Shared/utils/filesystem.h"

namespace fs = std::filesystem;

namespace AikariFileSystem
{
    // --- Begin implementations of FileSystemManager --- //

    FileSystemManager::FileSystemManager()
        : hugoAuraRootDir(
              AikariShared::Utils::FileSystem::getProgramDataPath()
                  .parent_path()
          ),
          aikariRootDir(
              AikariShared::Utils::FileSystem::getAikariRootDir(true)
          ),
          aikariConfigDir(aikariRootDir / "config"),
          aikariLogDir(aikariRootDir / "log") {};

    bool FileSystemManager::ensureDirExists() const
    {
        try
        {
            if (!fs::exists(this->aikariRootDir))
            {
                CUSTOM_LOG_INFO(
                    "Aikari root directory not exists, creating it..."
                );
                fs::create_directories(this->aikariRootDir);
            }

            CUSTOM_LOG_INFO("Checking log dir...");
            if (!fs::exists(this->aikariLogDir))
            {
                fs::create_directory(this->aikariLogDir);
            }

            CUSTOM_LOG_INFO("Checking config dir...");
            if (!fs::exists(this->aikariConfigDir))
            {
                fs::create_directory(this->aikariConfigDir);
            }

            return true;
        }
        catch (const std::exception& err)
        {
            CUSTOM_LOG_ERROR(
                "Error initializing Aikari storage directories. Error: {}",
                err.what()
            );
            return false;
        }
    }
    // --- End implementations of FileSystemManager --- //
};  // namespace AikariFileSystem
