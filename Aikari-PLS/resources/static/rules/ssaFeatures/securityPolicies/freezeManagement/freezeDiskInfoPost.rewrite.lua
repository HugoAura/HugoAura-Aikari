-- Rule Prop Area --

local ruleProp = {
    ruleType = "REWRITE",
    endpointType = "POST",
    featureType = "METHOD",
    featureName = "thing.event.freezeDiskInfo.post",
    ruleName = nil,
    ruleSide = "REQ",
    enabledBy = "ssaFeatures.securityPolicies.freezeManagement.freezeDiskInfoPost.enabled",
    configKey = "ssaFeatures.securityPolicies.freezeManagement.freezeDiskInfoPost"
}

-- Impl Area --

function onRecv(packet, config)

end
