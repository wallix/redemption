local utils = require'utils'

-- checks that all vcfg::var are used

local patternVarFile, patternVar, patternVarSearch
do
    local peg = utils.peg
    local C = peg.C
    local R = peg.R
    local Ct = peg.Ct
    local After = peg.After
    local Ident = 'cfg:' * (peg.word + ':')^1
    local ws = peg.ws
    local ws0 = peg.ws0
    patternVarFile = After('using' * ws * C(peg.word) * ws0 * '=' * ws0 * 'vcfg::variables<')
    patternVar = Ct(After('vcfg::var<' * C(Ident))^0)
    patternVarSearch = Ct(After('<' * C(Ident) * '>'+ peg.singleLineComment + peg.multiLineComment)^0)
end

local match_and_setk = utils.match_and_setk

local types, contents = {}, {}, {}

function file(content, filename)
    local type = patternVarFile:match(content)
    if type then
        types[type] = {filename=filename, values=match_and_setk(patternVar, content, false)}
    end

    contents[filename] = content

    -- match_and_setk(patternVarFile, content, true, values)
end

function terminate()
    for type,t in pairs(types) do
        local filename = t.filename
        local values = t.values

        local datas = {}

        for f,content in pairs(contents) do
            if content:find(type) then
                datas[f] = content
                local offset = filename:find('.hpp')
                -- append .cpp content if exists
                if offset then
                    f = filename:sub(0,offset)..'cpp'
                    local content = contents[f]
                    if content then
                        datas[f] = content
                    end
                end
            end
        end

        for _,content in pairs(datas) do
            match_and_setk(patternVarSearch, content, true, values)
            if content:find('%blanguage%(vars') then
                values['cfg::translation::language'] = true
            end
            if content:find('%blogin_language%(vars') then
                values['cfg::translation::login_language'] = true
            end
        end
    end

    utils.copy_setk(types['SessionProbeVariables'].values, types['ModRdpVariables'].values)

    local r = 0
    for type,t in pairs(types) do
        r = r + utils.count_error(t.values, t.filename .. ": %s not used with " .. type)
    end

    return r
end

return {init=function()end, file=file, terminate=terminate}
