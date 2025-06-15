#pragma once

namespace AikariTypes::global::lifecycle
{
enum class APPLICATION_RUNTIME_MODES
{
    NORMAL,
    DEBUG,
};

struct GlobalLifecycleStates
{
    APPLICATION_RUNTIME_MODES runtimeMode;
    long long launchTime;

    static GlobalLifecycleStates createDefault()
    {
        return {.runtimeMode = APPLICATION_RUNTIME_MODES::NORMAL, .launchTime = 0};
    }
};
}  // namespace AikariTypes::global::lifecycle
