local utils = require'utils'

module "config_checker"

local patternType, patternTypeSearch, patternVar, patternVarSearch
do
    local peg = utils.peg
    local C = peg.C
    local R = peg.R
    local Ct = peg.Ct
    local After = peg.After
    local Ident = 'cfg:' * (peg.word + ':')^1
    local SingleLineComment = '//' * After('\n')
    local MultiLineComment = '/*' * After('*/')
    patternVar = Ct(After(C(Ident))^0)
    patternVarSearch = Ct(After('<' * C(Ident) * '>'+ SingleLineComment + MultiLineComment)^0)
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
    values['cfg::icap_server_down::enable_x_context'] = true
    values['cfg::icap_server_down::filename_percent_encoding'] = true
    values['cfg::icap_server_down::host'] = true
    values['cfg::icap_server_down::port'] = true
    values['cfg::icap_server_down::service_name'] = true
    values['cfg::icap_server_down::tls'] = true
    values['cfg::icap_server_up::enable_x_context'] = true
    values['cfg::icap_server_up::filename_percent_encoding'] = true
    values['cfg::icap_server_up::host'] = true
    values['cfg::icap_server_up::port'] = true
    values['cfg::icap_server_up::service_name'] = true
    values['cfg::icap_server_up::tls'] = true
    -- translation
    values['cfg::translation::password_fr'] = true
    values['cfg::translation::password_en'] = true
end

function file(content)
    match_and_setk(patternVarSearch, content, true, values)
    match_and_setk(patternTypeSearch, content, true, types)
end

function terminate()
    return utils.count_error(values, "%s not used")
         + utils.count_error(types, "%s not used")
end
