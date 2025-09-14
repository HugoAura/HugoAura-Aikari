#ifdef AIKARIPLS_EXPORTS
#define AIKARIPLS_API __declspec(dllexport)
#else
#define AIKARIPLS_API __declspec(dllimport)
#endif

#include <Aikari-PLS/types/entrypoint.h>
#include <Aikari-Shared/types/itc/shared.h>
#include <filesystem>

namespace AikariPLS::Exports
{
    extern AIKARIPLS_API AikariPLS::Types::Entrypoint::EntrypointRet main(
        const std::filesystem::path& aikariRootPath,
        const std::filesystem::path& certDirPath,
        std::shared_ptr<
            AikariShared::Infrastructure::MessageQueue::SinglePointMessageQueue<
                AikariShared::Types::InterThread::MainToSubMessageInstance>>
            inputMessageQueue
    );

    extern AIKARIPLS_API void onExit();
}  // namespace AikariPLS::Exports
