#include <Aikari-Shared/utils/random.h>
#include <random>

namespace AikariShared::Utils::Random
{
    std::string genUUIDv4()
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> distribution(0, 255);

        std::stringstream ss;
        int idx;
        ss << std::hex;

        for (idx = 0; idx < 8; idx++)
        {
            ss << std::setw(2) << std::setfill('0') << distribution(gen);
        }
        ss << "-";
        for (idx = 0; idx < 4; idx++)
        {
            ss << std::setw(2) << std::setfill('0') << distribution(gen);
        }
        ss << "-4";

        for (idx = 0; idx < 3; idx++)
        {
            ss << std::setw(2) << std::setfill('0') << distribution(gen);
        }
        ss << "-";

        ss << std::setw(2) << std::setfill('0')
           << ((distribution(gen) & 0x3f) | 0x80);

        for (idx = 0; idx < 3; idx++)
        {
            ss << std::setw(2) << std::setfill('0') << distribution(gen);
        }
        ss << "-";
        for (idx = 0; idx < 12; idx++)
        {
            ss << std::setw(2) << std::setfill('0') << distribution(gen);
        }

        return ss.str();
    };
}  // namespace AikariShared::Utils::Random
