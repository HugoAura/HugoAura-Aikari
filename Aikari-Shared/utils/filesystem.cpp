#include <Aikari-Shared/utils/filesystem.h>  // self

#define CUSTOM_LOG_HEADER "[Shared / FileSystem Utils]"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <boost/algorithm/string.hpp>
#include <format>
#include <stdexcept>
#include <string>

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
                    continue;
                auto fileExt = perFile.path().extension().string();
                boost::algorithm::to_lower(fileExt);
                if (fileExt == extToMatch)
                {
                    (*resultTarget).emplace_back(perFile.path());
#ifdef _DEBUG
                    CUSTOM_LOG_TRACE(
                        "Found match path: {}", perFile.path().string()
                    );
#endif
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
}  // namespace AikariShared::Utils::FileSystem