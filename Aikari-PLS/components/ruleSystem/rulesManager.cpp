#include "./rulesManager.h"

#define CUSTOM_LOG_HEADER "[Rule Manager]"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/utils/filesystem.h>
#include <filesystem>

namespace AikariPLS::Components::Rules
{
    Manager::Manager(
        const std::filesystem::path& ruleDir, nlohmann::json& config
    )
        : ruleDir(ruleDir), config(config)
    {
    }

    Manager::~Manager()
    {
        /*
        this->scriptRuntime.reset(nullptr);
        this->scriptContext.reset(nullptr);*/
    }

    bool Manager::loadRules()
    {
        if (this->isRuleLoaded)
        {
            CUSTOM_LOG_WARN(
                "Rules has been already loaded, cannot call loadRules() again."
            );
            return false;
        }
        /*
        this->scriptRuntime.reset(nullptr);
        this->scriptContext.reset(nullptr);
        CUSTOM_LOG_INFO("Initializing JavaScript runtime...");
        this->scriptRuntime = std::make_unique<qjs::Runtime>();
        this->scriptContext =
            std::make_unique<qjs::Context>(*this->scriptRuntime.get());*/
        std::vector<std::filesystem::path> ruleFiles;
        CUSTOM_LOG_DEBUG(
            "Searching folder {} for scripts...", this->ruleDir.string()
        );
        AikariShared::Utils::FileSystem::grepFilesWithExt(
            this->ruleDir, Rules::FILE_EXT, &ruleFiles
        );
#ifdef _DEBUG
        {
            auto string_views =
                ruleFiles | std::views::transform(
                                [](const std::filesystem::path& p)
                                {
                                    return p.string();
                                }
                            );
            const std::string_view delimiter = ", ";
            std::string result;
            auto it = string_views.begin();
            if (it != string_views.end())
            {
                result += *it;
                ++it;
                for (; it != string_views.end(); ++it)
                {
                    result += delimiter;
                    result += *it;
                }
            }
            CUSTOM_LOG_DEBUG("Result:\n{}", result);
        }
#endif
        CUSTOM_LOG_INFO("Start loading scripts...");
        for (const auto& ruleFile : ruleFiles)
        {
            std::string fileContent;
            try
            {
                fileContent =
                    AikariShared::Utils::FileSystem::readFile(ruleFile);
            }
            catch (const std::exception& err)
            {
                CUSTOM_LOG_ERROR(
                    "Failed reading file: {} | Error: {}",
                    ruleFile.string(),
                    err.what()
                );
                continue;
            }
            /*
            this->scriptContext->eval(fileContent);
#ifdef _DEBUG
            CUSTOM_LOG_TRACE(
                "Script prop: {}",
                this->scriptContext->eval("ruleProp").toJSON()
            );
#endif
*/
        }
        return true;
    }

}  // namespace AikariPLS::Components::Rules