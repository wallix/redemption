local utils = require'utils'

local pattern
do
    local peg = utils.peg
    local Ident = 'ERR_' * (peg.R('AZ','09')+ '_')^1
    local op = peg.P'==' + '!='
    local left = op * peg.ws0 * 1
    local right = peg.ws0 * op
    pattern = peg.Ct(peg.After(left + peg.C(Ident) * (1-right))^0)
end

local match_and_setk = utils.match_and_setk

local error_file_hpp = 'src/core/error.hpp'
local error_files = {
    [error_file_hpp]=true,
    ['src/core/error.cpp']=true,
}

local kvalues = {}

return {
    init=function(args)
        kvalues = match_and_setk(pattern, utils.readall(error_file_hpp), false)
    end,

    file=function(content, filename)
        if not error_files[filename] then
            match_and_setk(pattern, content, true, kvalues)
        end
    end,

    terminate=function()
        return utils.count_error(kvalues, "%s not used")
    end,
}
