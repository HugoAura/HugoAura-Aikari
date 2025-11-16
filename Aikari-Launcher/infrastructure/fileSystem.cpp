#include "fileSystem.h"

#define CUSTOM_LOG_HEADER "[FS Manager]"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/utils/string.h>
#include <Aikari-Shared/utils/windows/winString.h>
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
                CUSTOM_LOG_DEBUG(
                    "Get ProgramData path success: {}", programDataPath.string()
                );

                finalPath = programDataPath;
            }
            catch (const fs::filesystem_error& error)
            {
                CUSTOM_LOG_ERROR(
                    "<!> A filesystem error occurred getting the path of "
                    "ProgramData dir."
                );
                LOG_ERROR(error.what());
                LOG_ERROR("Using the default value.");
                finalPath = fs::path("C:") / "ProgramData";
            }

            CoTaskMemFree(pathPtr);
        }
        else
        {
            CUSTOM_LOG_ERROR(
                "<!> Unexpected error occurred getting the path of ProgramData "
                "dir, trying to use default val."
            );
            LOG_ERROR("Error detail: ");
            LOG_ERROR(
                AikariShared::Utils::Windows::WinString::parseHResult(hResult)
            );
            finalPath = fs::path("C:") / "ProgramData";
        }

        return finalPath;
    };

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
