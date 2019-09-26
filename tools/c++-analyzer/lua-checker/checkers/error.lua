local utils = require'utils'

module "error_checker"

local pattern
do
    local peg = utils.peg
    local Ident = 'ERR_' * (peg.R('AZ','09')+ '_')^1
    pattern = peg.Ct(peg.After(peg.C(Ident))^0)
end

local match_and_setk = utils.match_and_setk

local error_file = 'src/core/error.hpp'

local values = {}
function init(args)
    values = match_and_setk(pattern, utils.readall(error_file), false)
end

function file(content, filename)
    if filename ~= error_file then
        match_and_setk(pattern, content, true, values)
    end
end

function terminate()
    return utils.count_error(values, "%s not used")
end
