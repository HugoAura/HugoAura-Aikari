#pragma once

#include <Aikari-Launcher-Private/types/global/lifecycleTypes.h>

#include <mutex>

namespace AikariLifecycle
{
class AikariStatesManager
{
   public:
    static AikariStatesManager& getInstance()
    {
        static AikariStatesManager instance;
        return instance;
    };

    const AikariTypes::global::lifecycle::GlobalLifecycleStates& getState()
    {
        std::lock_guard<std::mutex> lock(this->_mutex_lock_);
        return this->_state_;
    }

    template <typename T>
    void setVal(T AikariTypes::global::lifecycle::GlobalLifecycleStates::* key, const T& value)
    {
        std::lock_guard<std::mutex> lock(this->_mutex_lock_);
        this->_state_.*key = value;
    }

    template <typename T>
    T getVal(T AikariTypes::global::lifecycle::GlobalLifecycleStates::* key)
    {
        std::lock_guard<std::mutex> lock(this->_mutex_lock_);
        return this->_state_.*key;
    }

   private:
    AikariStatesManager() : _state_{AikariTypes::global::lifecycle::GlobalLifecycleStates::createDefault()} {};
    ~AikariStatesManager() = default;
    AikariStatesManager(const AikariStatesManager&) = delete;
    AikariStatesManager& operator=(const AikariStatesManager&) = delete;

    std::mutex _mutex_lock_;
    AikariTypes::global::lifecycle::GlobalLifecycleStates _state_;
};
}  // namespace AikariLifecycle
