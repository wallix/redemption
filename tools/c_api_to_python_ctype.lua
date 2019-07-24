local re = require're'

local typemap = {
    ['void*']='c_void_p',
    ['char*']='c_char_p',
    ['int']='c_int',
    ['unsigned']='c_uint',
    ['int8_t']='c_int8',
    ['int16_t']='c_int16',
    ['int32_t']='c_int32',
    ['int64_t']='c_int64',
    ['uint8_t']='c_uint8',
    ['uint16_t']='c_uint16',
    ['uint32_t']='c_uint32',
    ['uint64_t']='c_uint64',
}

function c2py(t)
    local type = t[2] and t[1] .. t[2] or t[1]
    local ctype = typemap[type]
    if not ctype then
        if t[2] then
            ctype = 'c_void_p'
        else
            error("unknown type " .. type)
        end
    end
    return ctype
end

prefix=''
defs={
    using=function(t)
        typemap[t[1]] = c2py(t[2])
        end,
    func=function(t)
        local types = {}
        for _k,p in ipairs(t[3]) do
            types[#types+1] = c2py(p)
        end
        print(prefix .. t[2] .. '.argtypes = [' .. table.concat(types, ', ') .. ']')
        print(prefix .. t[2] .. '.restype = ' .. c2py(t[1]))
        print()
    end,
}

local p = [=[

pattern     <- (S (function / using / endexpr))+
function    <- "REDEMPTION_LIB_EXPORT" ws {| type {id} '(' S params S ')' |} -> func
params      <- {| (param (',' S)?)* |}
param       <- S type S id S
using       <- "using " {| {id} S '=' S type |} -> using
id          <- [_a-zA-Z0-9]+
type        <- {| ('const' ws)? {id} (ws 'const')? S {'*'?} S |}
endexpr     <- comment1 / comment2 / [^;]+ ';'? / ';'
comment1    <- '//' [^%nl]* %nl
comment2    <- '/*' !'*/'* '*/'

ws          <- %s+
S           <- %s*

]=]

if not arg[1] then
    io.stderr:write('Usage:\n' .. arg[0] .. ' <file.h> [prefix]\n')
    return 1
end

prefix = arg[2] or ''

f,e=io.open(arg[1])
if e then
    error(e)
end
g=re.compile(p,defs)
if not g:match(f:read('*a')) then
    error('parsing error')
end
