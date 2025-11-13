local arrayUtils = {}

function arrayUtils.includes(array, val)
    if not array then return false end
    for _, v in ipairs(array) do
        if v == val then return true end
    end
    return false
end

return arrayUtils
