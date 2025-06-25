#pragma once

#include <string>

namespace AikariLauncherPublic::virtualIns
{
struct IConfigPayload
{
    std::string module;

    virtual ~IConfigPayload() = default;
};
}  // namespace AikariLauncherPublic::virtualIns
