#pragma once

#include <string>

namespace AikariShared::utils::cryptoUtils
{
std::string genRandomHexSecure(size_t& length);

std::string genRandomHexInsecure(size_t& length);
};  // namespace AikariShared::utils::cryptoUtils
