#pragma once

#include <windows.h>

#include <atomic>

namespace AikariTypes::global::lifecycle
{
enum class APPLICATION_RUNTIME_MODES
{
    NORMAL,
    DEBUG,
    SERVICE,
};

struct GlobalLifecycleStates
{
    APPLICATION_RUNTIME_MODES runtimeMode;
    long long launchTime;
    SERVICE_STATUS svcStatus;
    SERVICE_STATUS_HANDLE svcStatusHandle;
    std::atomic<bool> svcIsRunning;

    static GlobalLifecycleStates createDefault()
    {
        return {.runtimeMode = APPLICATION_RUNTIME_MODES::NORMAL,
                .launchTime = 0,
                .svcStatus = {0},
                .svcStatusHandle = NULL,
                .svcIsRunning = false};
    }
};
}  // namespace AikariTypes::global::lifecycle
