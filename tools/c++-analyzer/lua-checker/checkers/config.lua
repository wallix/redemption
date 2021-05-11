local utils = require'utils'

local patternType, patternTypeSearch, patternVar, patternVarSearch
do
    local peg = utils.peg
    local C = peg.C
    local R = peg.R
    local S = peg.S
    local P = peg.P
    local Ct = peg.Ct
    local After = peg.After
    local Ident = 'cfg:' * (peg.word + ':')^1
    patternVar = Ct(After(C(Ident))^0)
    patternVarSearch = Ct(After( '<' * C(Ident) * S'>'
                               + '(' * C(Ident) * (P'()' + '{}')
                               )^0)
    patternType = Ct((After('.enumeration_') * After('"') * C(peg.Until('"')))^0)
    patternTypeSearch = Ct(After((1-peg.wordchars) * C((R'AZ' * R('AZ','az')^1)))^0)
end

local match_and_setk = utils.match_and_setk

local values, types
function init(args)
    types = match_and_setk(patternType, utils.readall('projects/redemption_configs/configs_specs/configs/specs/config_type.hpp'), false)
    types['OcrLocale'] = true
    types['ColorDepth'] = true
    types['KeyboardLogFlagsCP'] = true

    values = match_and_setk(patternVar, utils.readall('projects/redemption_configs/autogen/include/configs/autogen/variables_configuration.hpp'), false)
    -- not used by rdpproxy...
    values['cfg::mod_rdp::krb_armoring_account'] = true
    values['cfg::mod_rdp::krb_armoring_fallback_password'] = true
    values['cfg::mod_rdp::krb_armoring_fallback_user'] = true
    values['cfg::mod_rdp::krb_armoring_realm'] = true
end

function file(content)
    match_and_setk(patternVarSearch, content, true, values)
    match_and_setk(patternTypeSearch, content, true, types)
end

function terminate()
    return utils.count_error(values, "%s not used")
         + utils.count_error(types, "%s not used")
end

return {init=init, file=file, terminate=terminate}
