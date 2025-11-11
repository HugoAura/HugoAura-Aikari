-- Rule Prop Area --

local cjsonUtil = require("cjson")

ruleProp = {
    ruleType = "REWRITE",
    endpointType = "POST",
    featureType = "METHOD",
    featureName = "thing.event.freezeDiskInfo.post",
    ruleName = nil,
    ruleSide = "REQ",
    enabledBy = "enabled",
    configKey = "ssaFeatures.securityPolicies.freezeManagement.freezeDiskInfoPost"
}

-- Impl Area --

function onRecv(packet, config)
    local pktAsTable, err = cjsonUtil.decode(packet)
    if not pktAsTable then
        print("Error decoding JSON: " .. err)
        return packet
    end
    pktAsTable.testFlag = true
    return cjsonUtil.encode(pktAsTable)
end
