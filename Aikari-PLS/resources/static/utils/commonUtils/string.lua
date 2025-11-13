local stringUtils = {}

local cjsonUtils = require("cjson")

function stringUtils.handleFnArgResolve(packet, config)
    local pktAsTable, pktErr = cjsonUtils.decode(packet)
    local configAsTable, cfgErr = cjsonUtils.decode(config)
    if not pktAsTable or not configAsTable then
        local errSide = pktAsTable and "config" or "pkt"
        print("Error decoding " .. errSide .. " JSON: " .. (pktErr or cfgErr))
        return nil, nil
    end
    return pktAsTable, configAsTable
end

function stringUtils.encodeTableToStringifyJSON(data)
    return cjsonUtils.encode(data)
end

return stringUtils
