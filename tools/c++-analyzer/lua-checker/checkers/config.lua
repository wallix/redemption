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
    local Ident = (peg.word + '::')^1

    patternVar = Ct(After(Ct(
        'struct ' * C(Ident)
      * ' {\n        static constexpr unsigned sesman_proxy_communication_flags = ' * C(peg.Until(';'))
    ))^0)

    patternVarSearch = Ct(After( Ct(C(P'get_mutable_ref' + 'get' + 'set_acl' + 'set' + 'ask' + 'send')
                                    * '<cfg::' * C(Ident) * S'>')
                               + '(cfg::' * C(Ident) * (P'()' + '{}')
                               )^0)

    patternType = Ct((After('.enumeration_') * After('"') * C(peg.Until('"')))^0)
    patternTypeSearch = Ct(After((1-peg.wordchars) * C((R'AZ' * R('AZ','az')^1)))^0)
end

local config_type_path = 'projects/redemption_configs/configs_specs/configs/specs/config_type.hpp'
local variables_configuration_path = 'projects/redemption_configs/autogen/include/configs/autogen/variables_configuration.hpp'

local ask_only = {
    ['context::keepalive']=true,
    -- TODO check that
    ['context::selector']=true,
    ['context::password']=true,
    -- TODO remove that
    ['context::nla_password_hash']=true,
    ['globals::nla_auth_user']=true,
}

local match_and_setk = utils.match_and_setk

-- {used:bool, sesman_to_proxy:bool, proxy_to_sesman:bool, get:bool, set:bool, ask:bool}
local values = {}
local types

function init(args)
    types = match_and_setk(patternType, utils.readall(config_type_path), false)
    types['OcrLocale'] = true
    types['ColorDepth'] = true
    types['KeyboardLogFlagsCP'] = true
    types['VncTunnelingType'] = true
    types['VncTunnelingCredentialSource'] = true

    for _,v in ipairs(patternVar:match(utils.readall(variables_configuration_path))) do
        values[v[1]] = {
            used=false,
            is_sesman_to_proxy = v[2]:byte(3) == 49 --[[ '1' ]],
            is_proxy_to_sesman = v[2]:byte(2) == 49 --[[ '1' ]],
        }
    end
end

function file(content)
    local r = patternVarSearch:match(content)
    local t
    if r then
        for _,v in ipairs(r) do
            if v[1] then
                t = values[v[2]]
                t.used = true
                if v[1] == 'get' then
                    t.get = true
                elseif v[1] == 'ask' then
                    t.ask = true
                elseif v[1] == 'send' then
                    t.get = true
                    t.set = true
                elseif v[1] ~= 'get_mutable_ref' then
                    t.set = true
                end
            else
                values[v].used = true
            end
        end
    end
    match_and_setk(patternTypeSearch, content, true, types)
end

function terminate()
    local errors = {}
    for name,t in pairs(values) do
        if not t.used then
            errors[#errors+1] = name .. " not used"
        else
            if not t.get and t.is_sesman_to_proxy then
                if not ask_only[name] or not t.ask then
                    local real_type = t.is_sesman_to_proxy and t.is_proxy_to_sesman
                        and 'sesman_rw'
                        or 'sesman_to_proxy'
                    errors[#errors+1] = name .. " is " .. real_type .. " but never read (should be proxy_to_sesman)"
                end
            end
            if not t.set and t.is_proxy_to_sesman then
                errors[#errors+1] = name .. " is proxy_to_sesman but never write (should be sesman_to_proxy)"
            end
        end
    end
    if #errors ~= 0 then
        errors[#errors+1] = ''
        utils.print_error(table.concat(errors,'\n'))
    end
    return utils.count_error(types, "%s not used") + #errors
end

return {init=init, file=file, terminate=terminate}
