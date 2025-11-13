#include "mqttPacketUtils.h"

#include <Aikari-Shared/infrastructure/loggerMacro.h>
#include <Aikari-Shared/utils/string.h>
#include <regex>
#include <sol/sol.hpp>
#include <unordered_map>

#include "../components/mqtt/mqttLifecycle.h"

namespace mqttMsgQueueTypes = AikariPLS::Types::MQTTMsgQueue;

namespace AikariPLS::Utils::MQTTPacketUtils
{
    AikariPLS::Types::MQTTMsgQueue::PacketTopicProps getPacketProps(
        const std::string& topic
    )
    {
        const auto splitResult = AikariShared::Utils::String::split(topic, '/');
        // clang-format off
        // ["", "sys", "<productKey>", "<deviceId>", "<action>", "<side>", "<?id>"]
        // clang-format on
        if (splitResult.size() <= 4)
        {
            throw std::invalid_argument("Invalid topic: " + topic);
        }
        const std::string& action = splitResult.at(4);
        AikariPLS::Types::MQTTMsgQueue::PacketTopicProps result;
        result.productKey = splitResult.at(2);
        result.deviceId = splitResult.at(3);
        if (action == "rpc")
        {
            result.side =
                splitResult.at(5) == "request"
                    ? AikariPLS::Types::MQTTMsgQueue::PACKET_SIDE::REP
                    : AikariPLS::Types::MQTTMsgQueue::PACKET_SIDE::REQ;
            // rpc requests are triggered by server-side, req = REP
            result.endpointType =
                AikariPLS::Types::MQTTMsgQueue::PACKET_ENDPOINT_TYPE::RPC;
            result.msgId = splitResult.at(6);
        }
        else if (action == "thing")
        {
            result.side = AikariPLS::Types::MQTTMsgQueue::PACKET_SIDE::REQ;
            result.endpointType =
                AikariPLS::Types::MQTTMsgQueue::PACKET_ENDPOINT_TYPE::POST;
        }
        else
        {
            result.side =
                splitResult.at(5) == "response"
                    ? AikariPLS::Types::MQTTMsgQueue::PACKET_SIDE::REP
                    : AikariPLS::Types::MQTTMsgQueue::PACKET_SIDE::REQ;
            // ↑ reversed from rpc
            result.endpointType =
                AikariPLS::Types::MQTTMsgQueue::PACKET_ENDPOINT_TYPE::GET;
            result.msgId = splitResult.at(6);
        }
        return result;
    }

    std::string mergeTopic(
        AikariPLS::Types::MQTTMsgQueue::PacketTopicProps& props
    )
    {
        const std::string sharedHead =
            std::format("/sys/{}/{}", props.productKey, props.deviceId);
        switch (props.endpointType)
        {
            case AikariPLS::Types::MQTTMsgQueue::PACKET_ENDPOINT_TYPE::GET:
            {
                return std::format(
                    "{}/up/{}/{}",
                    sharedHead,
                    props.side == Types::MQTTMsgQueue::PACKET_SIDE::REQ
                        ? "request"
                        : "response",
                    props.msgId.value()
                );
            }
            case AikariPLS::Types::MQTTMsgQueue::PACKET_ENDPOINT_TYPE::POST:
            {
                return std::format("{}/thing/post", sharedHead);
            }
            case AikariPLS::Types::MQTTMsgQueue::PACKET_ENDPOINT_TYPE::RPC:
            {
                return std::format(
                    "{}/rpc/{}/{}",
                    sharedHead,
                    props.side == Types::MQTTMsgQueue::PACKET_SIDE::REQ
                        ? "request"
                        : "response",
                    props.msgId.value()
                );
            }
            default:
            {
                throw std::invalid_argument(
                    "Invalid packet prop endpointType: " +
                    Types::MQTTMsgQueue::to_string(props.endpointType)
                );
            }
        }
    }

    async_mqtt::packet_variant reconstructPacket(
        async_mqtt::packet_variant& oldPacket,
        const std::function<async_mqtt::packet_id_type()>& buildNewPacketIdFn,
        std::optional<std::string> topicNameForPublish,
        std::optional<std::string> newPayload
    )
    {
        auto& sharedMsgQueues =
            AikariPLS::Lifecycle::MQTT::PLSMQTTMsgQueues::getInstance();
        auto& packetIdMap = sharedMsgQueues.getVal(
            &AikariPLS::Types::Lifecycle::MQTT::PLSMQTTMsgQueues::packetIdMap
        );

        async_mqtt::packet_variant result(oldPacket);

        oldPacket.visit(
            async_mqtt::overload{
                [&](async_mqtt::v3_1_1::subscribe_packet const& pkt)
                {
                    auto newPacketId = buildNewPacketIdFn();
                    packetIdMap[newPacketId] = pkt.packet_id();

                    result = async_mqtt::v3_1_1::subscribe_packet(
                        newPacketId, pkt.entries()
                    );
                },
                [&](async_mqtt::v3_1_1::suback_packet const& pkt)
                {
                    auto newPacketId = packetIdMap[pkt.packet_id()];

                    result = async_mqtt::v3_1_1::suback_packet(
                        newPacketId, pkt.entries()
                    );
                },
                [&](async_mqtt::v3_1_1::unsubscribe_packet const& pkt)
                {
                    auto newPacketId = buildNewPacketIdFn();
                    packetIdMap[newPacketId] = pkt.packet_id();

                    result = async_mqtt::v3_1_1::unsubscribe_packet(
                        newPacketId, pkt.entries()
                    );
                },
                [&](async_mqtt::v3_1_1::unsuback_packet const& pkt)
                {
                    auto newPacketId = packetIdMap[pkt.packet_id()];

                    result = async_mqtt::v3_1_1::unsuback_packet(newPacketId);
                },
                [&](async_mqtt::v3_1_1::publish_packet const& pkt)
                {
                    auto newPacketId = buildNewPacketIdFn();

                    result = async_mqtt::v3_1_1::publish_packet(
                        newPacketId,
                        topicNameForPublish.value_or(pkt.topic()),
                        newPayload.value_or(pkt.payload()),
                        pkt.opts()
                    );
                },
                // Auto PUBACK is ON, so type !== PUBACK
                [&](auto const&)
                {
                    result = oldPacket;
                } }
        );

        sharedMsgQueues.setVal(
            &AikariPLS::Types::Lifecycle::MQTT::PLSMQTTMsgQueues::packetIdMap,
            packetIdMap
        );

        return result;
    };

    nlohmann::json _processPropertyData(
        const nlohmann::json& paramJson,
        const AikariPLS::Types::RuleSystem::RuleMapping::RewriteFeaturesStore&
            featureStore
    )
    {
        static const std::string logHeader =
            "[MQTT Utils] <Packet Processor - Prop>";
        nlohmann::json newParamPayload = paramJson;
        for (const auto& el : paramJson.items())
        {
            // ↑ iterate all params
            // ↓ chk if appears in featureStore.prop
            if (!featureStore.prop.contains(el.key()))
                continue;
            for (const auto& perRewriteRule : featureStore.prop.at(el.key()))
            {
                if (!perRewriteRule.isEnabled)
                    continue;

                sol::protected_function_result fnExecResult =
                    perRewriteRule.rewriteFn(
                        newParamPayload, perRewriteRule.config.dump()
                    );  // provide full payload["params"]
                bool isThisRunValid = false;
                if (fnExecResult.valid())
                {
                    sol::object fnExecResultObj = fnExecResult;
                    if (fnExecResultObj.is<std::string>())
                    {
                        newParamPayload = fnExecResultObj.as<std::string>();
                        isThisRunValid = true;
                    }
                }

                if (!isThisRunValid)
                {
                    LOG_ERROR(
                        "{} "
                        "Detected error while running Lua "
                        "function for REWRITE://PROP/{}, "
                        "please report this to Aikari GitHub "
                        "Issues",
                        logHeader,
                        el.key()
                    );
                    continue;
                }
            }
        }
        return newParamPayload;
    };

    RewritePacketProcessResult processPacketDataWithRule(
        const std::string& fullPayloadRaw,
        const AikariPLS::Types::RuleSystem::RuleMapping::RewriteFeaturesStore&
            featureStore,
        const AikariPLS::Types::MQTTMsgQueue::PACKET_ENDPOINT_TYPE& endpointType
    )
    {
        static const std::string logHeader = "[MQTT Utils] <Packet Processor>";
        const std::regex matchMethod(R"~("method"\s*:\s*"([^"]+)")~");
        std::string_view payloadStrView(fullPayloadRaw);

        std::match_results<std::string_view::const_iterator> match;

        RewritePacketMethodType methodType = RewritePacketMethodType::NORMAL;
        LOG_TRACE("{} Begin regex search", logHeader);
        if (std::regex_search(
                payloadStrView.cbegin(),
                payloadStrView.cend(),
                match,
                matchMethod
            ))
        {
            LOG_TRACE("{} End regex search, found", logHeader);
            std::string methodName{ match[1] };
            if (methodName ==
                AikariPLS::Utils::MQTTPacketUtils::propTypeGetMethodName)
            {
                methodType = RewritePacketMethodType::PROP_GET;
                return { .packetMethodType = methodType };
            }
            else if (methodName ==
                     AikariPLS::Utils::MQTTPacketUtils::propTypeSetMethodName)
            {
                methodType = RewritePacketMethodType::PROP_SET;
                nlohmann::json payloadAsJson;
                try
                {
                    payloadAsJson = nlohmann::json::parse(fullPayloadRaw);
                }
                catch (...)
                {
                    LOG_WARN(
                        "{} Failed to parse prop set payload as JSON, "
                        "payload:\n{}",
                        logHeader,
                        fullPayloadRaw
                    );
                    return { .packetMethodType = methodType,
                             .newPayload = std::nullopt };
                }
                nlohmann::json& setPropParams = payloadAsJson["params"];
                nlohmann::json newPayload = payloadAsJson;
                newPayload["params"] =
                    _processPropertyData(setPropParams, featureStore);
                if (newPayload == payloadAsJson)
                {
                    return { .packetMethodType = methodType,
                             .newPayload = std::nullopt };
                }
                else
                {
                    return { .packetMethodType = methodType,
                             .newPayload = newPayload.dump() };
                }
            }
            else  // Method name == any other of property.set / property.get
            {
                methodType = RewritePacketMethodType::NORMAL;
                auto methodMapIterator = featureStore.method.find(endpointType);
                if (methodMapIterator == featureStore.method.end())
                    return {
                        .packetMethodType = methodType,
                        .newPayload = std::nullopt
                    };  // if method[GET/POST/RPC] not exists

                const AikariPLS::Types::RuleSystem::RuleMapping::
                    RewriteFeaturesMap& targetFeaturesMap =
                        methodMapIterator->second;

                auto featureMapIterator = targetFeaturesMap.find(methodName);
                if (featureMapIterator == targetFeaturesMap.end())
                    return { .packetMethodType = methodType,
                             .newPayload = std::nullopt };

                auto rewriteRulePropVec = featureMapIterator->second;
                std::string curNewPayload;
                for (const auto& perRewriteRule : rewriteRulePropVec)
                {
                    if (!perRewriteRule.isEnabled)
                        continue;

                    if (curNewPayload.empty())
                        curNewPayload = fullPayloadRaw;

                    sol::protected_function_result fnExecResult =
                        perRewriteRule.rewriteFn(
                            fullPayloadRaw, perRewriteRule.config.dump()
                        );
                    bool fnRepValid = false;

                    if (fnExecResult.valid())
                    {
                        sol::object fnExecResultObj = fnExecResult;
                        if (fnExecResultObj.is<std::string>())
                        {
                            fnRepValid = true;
                            curNewPayload = fnExecResultObj.as<std::string>();
                        }
                    }

                    if (!fnRepValid)
                    {
                        sol::error err = fnExecResult;
                        LOG_ERROR(
                            "{} "
                            "Detected error while running Lua "
                            "function for REWRITE://{}/{}, "
                            "please report this to Aikari GitHub "
                            "Issues.\nError: {}",
                            logHeader,
                            AikariPLS::Types::MQTTMsgQueue::to_string(
                                endpointType
                            ),
                            methodName,
                            err.what()
                        );
                    }
                }

                if (curNewPayload.empty() || curNewPayload == fullPayloadRaw)
                {
                    return { .packetMethodType = methodType,
                             .newPayload = std::nullopt };
                }
                else
                {
                    return { .packetMethodType = methodType,
                             .newPayload = curNewPayload };
                }
            }
        }

        LOG_TRACE("{} End regex search, not found", logHeader);
        return { .packetMethodType = methodType, .newPayload = std::nullopt };
    };

    std::optional<std::string> processPropGetRepPacket(
        const std::string& fullPayloadRaw,
        const AikariPLS::Types::RuleSystem::RuleMapping::RewriteFeaturesStore&
            featureStore
    )
    {
        static const std::string logHeader = "[MQTT Utils] <GET PROP REP>";
        nlohmann::json payloadAsJson;
        try
        {
            payloadAsJson = nlohmann::json::parse(fullPayloadRaw);
        }
        catch (...)
        {
            LOG_ERROR(
                "{} Failed to parse payload as JSON, payload:\n",
                logHeader,
                fullPayloadRaw
            );
            return std::nullopt;
        }
        nlohmann::json newPayload = payloadAsJson;
        newPayload["data"] =
            _processPropertyData(payloadAsJson["data"], featureStore);
        if (newPayload == payloadAsJson)
        {
            return std::nullopt;
        }
        else
        {
            return newPayload.dump();
        }
    };
}  // namespace AikariPLS::Utils::MQTTPacketUtils
