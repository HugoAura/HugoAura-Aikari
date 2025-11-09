#include <Aikari-PLS-Private/types/components/mqtt.h>

namespace AikariPLS::Types::MQTTMsgQueue
{
    std::string to_string(PACKET_ENDPOINT_TYPE endpointType)
    {
        switch (endpointType)
        {
            case PACKET_ENDPOINT_TYPE::GET:
            {
                return "GET";
            }
            case PACKET_ENDPOINT_TYPE::POST:
            {
                return "POST";
            }
            case PACKET_ENDPOINT_TYPE::RPC:
            {
                return "RPC";
            }
            default:
            {
                return "UNKNOWN";
            }
        }
    };

    std::string to_string(PACKET_SIDE pktSide)
    {
        switch (pktSide)
        {
            case PACKET_SIDE::REP:
            {
                return "REP";
            }
            case PACKET_SIDE::REQ:
            {
                return "REQ";
            }
            default:
            {
                return "UNKNOWN";
            }
        }
    };
}  // namespace AikariPLS::Types::MQTTMsgQueue
