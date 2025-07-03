#include "pch.h"

#include <Aikari-PLS-Private/types/lifecycleTypes.h>

#include "infrastructure/threadMsgHandler.h"

namespace AikariPLS::Types::lifecycle
{
    PLSSharedIns::PLSSharedIns() = default;
    PLSSharedIns PLSSharedIns::createDefault()
    {
        return {};
    };
    PLSSharedIns::~PLSSharedIns() = default;
    PLSSharedIns::PLSSharedIns(PLSSharedIns&&) noexcept = default;
    PLSSharedIns& PLSSharedIns::operator=(PLSSharedIns&&) noexcept = default;
}  // namespace AikariPLS::Types::lifecycle
