-- Rule Prop Area --

local cjsonUtils = require("cjson")
local stringUtils = require("commonUtils/string")

ruleProp = {
    ruleType = "REWRITE",
    endpointType = "POST",
    featureType = "METHOD",
    featureName = "thing.event.softwareReport.post",
    ruleName = nil,
    ruleSide = "REQ",
    enabledBy = "enabled",
    configKey = "deviceInfo.software.softwareReportPost"
}

-- Impl Area --

function onRecv(packet, config)
    local pktAsTable, configAsTable = stringUtils.handleFnArgResolve(packet, config)
    if not pktAsTable or not configAsTable then return packet end
    if configAsTable.setAsEmpty then
        pktAsTable.params.lists = {}
    end
    return stringUtils.encodeTableToStringifyJSON(pktAsTable)
end
