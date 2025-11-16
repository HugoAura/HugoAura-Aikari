#pragma once

#include <nlohmann/json.hpp>

#include "errors.h"

namespace AikariLauncher::Public::Constants::InterThread::Errors::Templates
{
    inline const nlohmann::json ROUTE_NOT_FOUND = {
        { "success", false },
        { "message", "Route not found" },
        { "diagnoseCode", Errors::ROUTE_NOT_FOUND }
    };
}
