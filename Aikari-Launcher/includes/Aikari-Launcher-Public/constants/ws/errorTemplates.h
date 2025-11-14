#pragma once

#include <nlohmann/json.hpp>

#include "../../types/components/wsTypes.h"
#include "errors.h"

namespace AikariLauncherPublic::Constants::WebSocket::Errors::Templates
{
    inline const AikariLauncherPublic::Types::Components::WebSocket::ServerWSRep
        MODULE_NOT_FOUND = { .code = Errors::Codes::MODULE_NOT_FOUND,
                             .success = false,
                             .data = {
                                 { "message",
                                   "Requested module not founded, are you "
                                   "running the latest version of HugoAura?" },
                                 { "diagnoseId", Errors::MODULE_NOT_FOUND } } };

    inline const AikariLauncherPublic::Types::Components::WebSocket::ServerWSRep INVALID_ARG = {
        .code = Errors::Codes::INVALID_ARG,
        .success = false,
        .data = { { "message", "Invalid arguments provided" },
                  { "diagnoseId", Errors::INVALID_ARG } }
    };

    inline const AikariLauncherPublic::Types::Components::WebSocket::ServerWSRep
        METHOD_NOT_FOUND = { .code = Errors::Codes::METHOD_NOT_FOUND,
                             .success = false,
                             .data = {
                                 { "message", "Method not found" },
                                 { "diagnoseId", Errors::METHOD_NOT_FOUND } } };
}  // namespace AikariLauncherPublic::Constants::WebSocket::Errors::Templates
