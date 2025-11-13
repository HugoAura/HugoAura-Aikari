-- Rule Prop Area --

local cjsonUtils = require("cjson")
local arrayUtils = require("commonUtils/array")
local stringUtils = require("commonUtils/string")

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
    local pktAsTable, configAsTable = stringUtils.handleFnArgResolve(packet, config)
    if not pktAsTable or not configAsTable then return packet end
    for _, thisDisk in ipairs(pktAsTable.params.diskInfoList) do
        local isFrozen = arrayUtils.includes(configAsTable.frozenDisks, thisDisk.diskName)
        if isFrozen then
            thisDisk.protectedStatus = 1
        else
            thisDisk.protectedStatus = 0
        end
    end
    return stringUtils.encodeTableToStringifyJSON(pktAsTable)
end
