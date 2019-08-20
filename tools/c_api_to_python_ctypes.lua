#!lua
re = require're'

typemap = {
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

imported = {}
lines = {}
prefix=''
prefix_ctypes=''

function c2py(t)
    local type = t[2] and t[1] .. t[2] or t[1]
    local ctype = typemap[type]
    if not ctype then
        if t[2] then
            ctype = prefix_ctypes..'c_void_p'
        else
            error("unknown type " .. type)
        end
    end
    return ctype
end

defs={
    using=function(t)
        typemap[t[1]] = c2py(t[2])
    end,
    typedef=function(t)
        typemap[t[2]] = c2py(t[1])
    end,
    func=function(t)
        local types = {}
        for _k,p in ipairs(t[3]) do
            local c_type = c2py(p)
            types[#types+1] = c_type
            imported[c_type] = true
        end
        lines[#lines+1] = prefix .. t[2] .. '.argtypes = [' .. table.concat(types, ', ') .. ']'
        lines[#lines+1] = prefix .. t[2] .. '.restype = ' .. c2py(t[1])
        lines[#lines+1] = ''
    end,
}

-- http://www.inf.puc-rio.br/~roberto/lpeg/re.html

-- TODO pointer to function
local p = [=[

pattern     <- (S (function / using / endexpr))+
function    <- "REDEMPTION_LIB_EXPORT" ws {| type {id} '(' S params S ')' |} -> func
params      <- {| (param (',' S)?)* |}
param       <- S type S id S
using       <- "using " {| {id} S '=' S type |} -> using
typedef     <- "typedef " {| type ws {id} |} -> typedef
id          <- [_a-zA-Z0-9]+
type        <- {| ('const' ws)? {id} (ws 'const')? S {'*'?} S |}
endexpr     <- comment1 / comment2 / [^;]+ ';'? / ';'
comment1    <- '//' [^%nl]* %nl
comment2    <- '/*' !'*/'* '*/'

ws          <- %s+
S           <- %s*

]=]

if not arg[1] then
    io.stderr:write('Usage:\n' .. arg[0] .. ' <file.h> [prefix_lib] [prefix_ctypes]\n')
    return 1
end

prefix = arg[2] or ''
if arg[3] then
    prefix_ctypes = arg[3]
    for k,v in pairs(typemap) do
        typemap[k] = prefix_ctypes .. v
    end
end

f,e=io.open(arg[1])
if e then
    error(e)
end
g=re.compile(p,defs)
if not g:match(f:read('*a')) then
    error('parsing error')
end

if #prefix_ctypes == 0 then
    local t = {}
    for k,_ in pairs(imported) do
        t[#t+1] = k
    end
    table.sort(t)
    print('from ctypes import ' .. table.concat(t, ', '))
    print()
end
print(table.concat(lines, '\n'))
