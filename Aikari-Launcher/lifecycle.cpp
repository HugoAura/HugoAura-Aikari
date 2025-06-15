#include <Aikari-Launcher-Private/common.h>
#include "lifecycle.h"

#include <Aikari-Launcher-Private/types/global/lifecycleTypes.h>

#include <mutex>

namespace AikariLifecycle
{
// --- Begin implementations of AikariStatesManager --- //
void AikariStatesManager::setState(const AikariTypes::global::lifecycle::GlobalLifecycleStates& newState)
{
    std::lock_guard<std::mutex> lock(this->_mutex_lock_);
    this->_state_ = newState;
}

AikariTypes::global::lifecycle::GlobalLifecycleStates AikariStatesManager::getState()
{
    std::lock_guard<std::mutex> lock(this->_mutex_lock_);
    return this->_state_;
}
// --- End implementations of AikariStatesManager --- //
}  // namespace AikariLifecycle
