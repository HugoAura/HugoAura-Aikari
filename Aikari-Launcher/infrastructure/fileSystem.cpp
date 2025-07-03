#include "pch.h"

#include "fileSystem.h"

#include <Aikari-Launcher-Private/common.h>
#include <Aikari-Shared/utils/string.h>
#include <ShlObj.h>
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;

namespace AikariFileSystem
{
    // --- Begin implementations of FileSystemManager --- //
    fs::path FileSystemManager::getProgramDataPath()
    {
        PWSTR pathPtr = NULL;
        HRESULT hResult =
            SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &pathPtr);
        fs::path finalPath;

        if (SUCCEEDED(hResult))
        {
            try
            {
                fs::path programDataPath(pathPtr);
                LOG_DEBUG(
                    "Get ProgramData path success: {}", programDataPath.string()
                );

                finalPath = programDataPath;
            }
            catch (const fs::filesystem_error& error)
            {
                LOG_ERROR(
                    "⚠ A filesystem error occurred getting the path of "
                    "ProgramData "
                    "dir."
                );
                LOG_ERROR(error.what());
                LOG_ERROR("Using the default value.");
                finalPath = fs::path("C") / "ProgramData";
            }

            CoTaskMemFree(pathPtr);
        }
        else
        {
            LOG_ERROR(
                "⚠ Unexpected error occurred getting the path of ProgramData "
                "dir, "
                "trying to use default val."
            );
            LOG_ERROR("Error detail: ");
            LOG_ERROR(AikariShared::utils::string::parseHResult(hResult));
            finalPath = fs::path("C") / "ProgramData";
        }

        return finalPath;
    };

    int FileSystemManager::ensureDirExists() const
    {
        if (!fs::exists(this->aikariRootDir))
        {
            LOG_INFO("Aikari root directory not exists, creating it...");
            fs::create_directories(this->aikariRootDir);
        }

        LOG_INFO("Checking log dir...");
        if (!fs::exists(this->aikariLogDir))
        {
            fs::create_directory(this->aikariLogDir);
        }

        LOG_INFO("Checking config dir...");
        if (!fs::exists(this->aikariConfigDir))
        {
            fs::create_directory(this->aikariConfigDir);
        }

        return 0;
    }
    // --- End implementations of FileSystemManager --- //
};  // namespace AikariFileSystem
