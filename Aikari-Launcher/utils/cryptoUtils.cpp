#include "cryptoUtils.h"

#include <iomanip>
#include <random>
#include <sstream>
#include <vector>

namespace AikariUtils::cryptoUtils
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
};  // namespace AikariUtils::cryptoUtils
