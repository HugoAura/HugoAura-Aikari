#include <Aikari-Shared/utils/filesystem.h>  // self

#define CUSTOM_LOG_HEADER "[Shared / FileSystem Utils]"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/utils/windows/winString.h>
#include <ShlObj.h>
#include <boost/algorithm/string.hpp>
#include <format>
#include <fstream>
#include <knownfolders.h>
#include <stdexcept>
#include <string>
#include <windows.h>

namespace AikariShared::Utils::FileSystem
{
    std::filesystem::path getSelfPathFromHandler(HMODULE hModule)
    {
        wchar_t path[MAX_PATH];

        DWORD result = GetModuleFileNameW(hModule, path, MAX_PATH);
        if (result == 0)
        {
            DWORD errorCode = GetLastError();
            throw std::runtime_error(
                std::format(
                    "Error getting selfPath from HMODULE, errorCode: {}",
                    std::to_string(errorCode)
                )
            );
        }
        else
        {
            std::filesystem::path pathObj(path);
#ifdef _DEBUG
            CUSTOM_LOG_TRACE("Get selfPath result: {}", pathObj.string());
#endif
            return pathObj;
        }
    };

    std::function<std::filesystem::path()> genGetSelfPathLambda(
        std::optional<HMODULE> hModule
    )
    {
        return [hModule]() -> std::filesystem::path
        {
            wchar_t path[MAX_PATH];

            DWORD result =
                GetModuleFileNameW(hModule.value_or(nullptr), path, MAX_PATH);
            if (result == 0)
            {
                DWORD errorCode = GetLastError();
                throw std::runtime_error(
                    std::format(
                        "Error getting selfPath from HMODULE, errorCode: {}",
                        std::to_string(errorCode)
                    )
                );
            }
            else
            {
                std::filesystem::path pathObj(path);
                return pathObj;
            }
        };
    }

    void grepFilesWithExt(
        const std::filesystem::path& dir,
        const std::string& extToMatch,
        std::vector<std::filesystem::path>* resultTarget
    )
    {
        if (resultTarget == nullptr)
        {
            throw std::invalid_argument("Result target vector is required.");
        }
        if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir))
        {
            for (const auto& perFile : std::filesystem::directory_iterator(dir))
            {
                if (perFile.is_directory())
                {
#ifdef _DEBUG
                    CUSTOM_LOG_TRACE(
                        "Diving into directory: {}", perFile.path().string()
                    );
#endif
                    FileSystem::grepFilesWithExt(
                        perFile.path(), extToMatch, resultTarget
                    );
                }
                if (!perFile.is_regular_file())
                {
                    continue;
                }
                auto fileExt = perFile.path().extension().string();
                boost::algorithm::to_lower(fileExt);
                if (fileExt == extToMatch)
                {
                    (*resultTarget).emplace_back(perFile.path());
                }
            }
        }
        else
        {
            throw std::invalid_argument(
                "Invalid dir arg: ENOENT or not a directory."
            );
        }
    };

    std::string readFile(const std::filesystem::path& path)
    {
        const std::ifstream file(path);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed opening file: " + path.string());
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::filesystem::path getProgramDataPath(bool showLog)
    {
        PWSTR pathPtr = NULL;
        HRESULT hResult =
            SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &pathPtr);
        std::filesystem::path finalPath;

        if (SUCCEEDED(hResult))
        {
            try
            {
                std::filesystem::path programDataPath(pathPtr);
                if (showLog)
                    CUSTOM_LOG_DEBUG(
                        "Get ProgramData path success: {}",
                        programDataPath.string()
                    );

                finalPath = programDataPath;
            }
            catch (const std::filesystem::filesystem_error& error)
            {
                if (showLog)
                {
                    CUSTOM_LOG_ERROR(
                        "<!> A filesystem error occurred getting the path of "
                        "ProgramData dir."
                    );
                    LOG_ERROR(error.what());
                    LOG_ERROR("Using the default value.");
                }
                finalPath = std::filesystem::path("C:") / "ProgramData";
            }

            CoTaskMemFree(pathPtr);
        }
        else
        {
            if (showLog)
            {
                CUSTOM_LOG_ERROR(
                    "<!> Unexpected error occurred getting the path of "
                    "ProgramData "
                    "dir, trying to use default val."
                );
                LOG_ERROR("Error detail: ");
                LOG_ERROR(
                    AikariShared::Utils::Windows::WinString::parseHResult(
                        hResult
                    )
                );
            }
            finalPath = std::filesystem::path("C:") / "ProgramData";
        }

        return finalPath;
    };

    std::filesystem::path getAikariRootDir(bool showLog)
    {
        try
        {
            return getProgramDataPath(showLog) / "HugoAura" / "Aikari";
        }
        catch (...)
        {
            return std::filesystem::path("C:") / "ProgramData" / "HugoAura" /
                   "Aikari";
        }
    };
}  // namespace AikariShared::Utils::FileSystem