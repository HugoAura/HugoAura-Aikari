#pragma once

#include <mutex>

namespace AikariShared::base
{
    template <typename StoreType>
    class AikariStatesManagerTemplate
    {
       public:
        static AikariStatesManagerTemplate& getInstance()
        {
            static AikariStatesManagerTemplate instance;
            return instance;
        };

        const StoreType& getState()
        {
            std::lock_guard<std::mutex> lock(this->_mutex_lock_);
            return this->_state_;
        }

        template <typename T>
        void setVal(T StoreType::* key, const T& value)
        {
            std::lock_guard<std::mutex> lock(this->_mutex_lock_);
            this->_state_.*key = value;
        }

        template <typename T>
        T getVal(T StoreType::* key)
        {
            std::lock_guard<std::mutex> lock(this->_mutex_lock_);
            return this->_state_.*key;
        }

        template <typename T>
        void setPtr(T StoreType::* key, T&& value)
        {
            std::lock_guard<std::mutex> lock(this->_mutex_lock_);
            this->_state_.*key = std::forward<T>(value);
        }

        template <typename T>
        typename T::element_type* getPtr(T StoreType::* key)
        {
            std::lock_guard<std::mutex> lock(this->_mutex_lock_);
            return (this->_state_.*key).get();
        }

        template <typename T>
        void resetPtr(T StoreType::* key)
        {
            std::lock_guard<std::mutex> lock(this->_mutex_lock_);
            (this->_state_.*key).reset();
        }

       private:
        AikariStatesManagerTemplate()
            : _state_{ StoreType::createDefault() } {};
        ~AikariStatesManagerTemplate() = default;
        AikariStatesManagerTemplate(const AikariStatesManagerTemplate&) =
            delete;
        AikariStatesManagerTemplate&
        operator=(const AikariStatesManagerTemplate&) = delete;

        std::mutex _mutex_lock_;
        StoreType _state_;
    };
}  // namespace AikariShared::base
