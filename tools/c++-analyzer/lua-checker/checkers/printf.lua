local utils = require'utils'

local pattern
do
    local peg = utils.peg
    pattern = peg.After(peg.P'%*s')
end

local kvalues = {}

return {
    init=function(args)
    end,

    file=function(content, filename)
        if pattern:match(content) then
            kvalues[filename] = false
        end
    end,

    terminate=function()
        return utils.count_error(kvalues, "%s contents %%*s instead of %%.*s in printf format")
    end,
}
