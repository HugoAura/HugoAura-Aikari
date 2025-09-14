#pragma once

#include <string>

namespace AikariShared::Utils::CryptoUtils
{
    std::string genRandomHexSecure(size_t length);

    std::string genRandomHexInsecure(size_t length);
};  // namespace AikariShared::Utils::CryptoUtils
