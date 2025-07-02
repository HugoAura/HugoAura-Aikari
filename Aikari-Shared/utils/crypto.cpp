#include "pch.h"

#include <Aikari-Shared/utils/crypto.h>
#include <format>
#include <iomanip>
#include <random>
#include <sstream>
#include <vector>

namespace AikariShared::utils::cryptoUtils
{
std::string genRandomHexSecure(size_t& length)
{
    if (length == 0 || length % 2 != 0)
    {
        throw std::invalid_argument(
            std::format("Invalid arg for genRandomHexSecure: length={}", length)
        );
    }

    size_t byteSize = length / 2;

    std::vector<unsigned char> randomBytesArr(byteSize);
    std::random_device randomDvc;

    std::uniform_int_distribution<unsigned int> dist(0, 255);

    for (size_t i = 0; i < byteSize; ++i)
    {
        randomBytesArr[i] = static_cast<unsigned char>(dist(randomDvc));
    }

    std::stringstream resultStream;
    resultStream << std::hex << std::setfill('0');

    for (const auto& byte : randomBytesArr)
    {
        resultStream << std::setw(2) << static_cast<int>(byte);
    }

    return resultStream.str();
}

std::string genRandomHexInsecure(size_t& length)
{
    std::stringstream strStream;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (size_t i = 0; i < length; ++i)
    {
        int value = std::rand() % 16;
        strStream << std::hex << value;
    }
    return strStream.str();
};
};  // namespace AikariShared::utils::cryptoUtils
