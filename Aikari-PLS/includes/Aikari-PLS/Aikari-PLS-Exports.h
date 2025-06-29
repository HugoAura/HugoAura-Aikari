#ifdef AIKARIPLS_EXPORTS
#define AIKARIPLS_API __declspec(dllexport)
#else
#define AIKARIPLS_API __declspec(dllimport)
#endif

#include <Aikari-PLS/types/entrypoint.h>
#include <Aikari-PLS/types/infrastructure/messageQueue.h>

namespace AikariPLS::Exports
{
extern AIKARIPLS_API AikariPLS::Types::entrypoint::EntrypointRet main(
    const std::filesystem::path& aikariRootPath,
    const std::filesystem::path& certDirPath,
    std::shared_ptr<
        AikariShared::infrastructure::MessageQueue::SinglePointMessageQueue<
            AikariPLS::Types::infrastructure::InputMessageStruct>>
        inputMessageQueue
);
}
