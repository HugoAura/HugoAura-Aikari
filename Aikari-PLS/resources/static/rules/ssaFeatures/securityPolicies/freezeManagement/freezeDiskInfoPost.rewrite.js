// Rule Prop Area

const ruleProp = {
    ruleType: "REWRITE",
    endpointType: "POST",
    featureType: "METHOD",
    featureName: "thing.event.freezeDiskInfo.post",
    ruleName: null,
    ruleSide: "REQ",
    enabledBy: "ssaFeatures.securityPolicies.freezeManagement.freezeDiskInfoPost.enabled"
}

// Impl Area

function onRecv(packet, config) {
}
