#!/usr/bin/env lua
re = require're'

typemap = {
    ['void*']='c_void_p',
    ['char*']='c_char_p',
    ['uint8_t*']='c_char_p',
    ['char']='c_char',
    ['int']='c_int',
    ['long']='c_long',
    ['long long']='c_longlong',
    ['unsigned']='c_uint',
    ['unsigned int']='c_uint',
    ['unsigned long']='c_ulong',
    ['unsigned long long']='c_ulonglong',
    ['int8_t']='c_int8',
    ['int16_t']='c_int16',
    ['int32_t']='c_int32',
    ['int64_t']='c_int64',
    ['uint8_t']='c_uint8',
    ['uint16_t']='c_uint16',
    ['uint32_t']='c_uint32',
    ['uint64_t']='c_uint64',
}

basetype = {POINTER=true}
for _,v in pairs(typemap) do
    basetype[v] = true
end
typemap['void'] = 'None'

imported = {}
lines = {}
prefix=''
prefix_ctypes=''

function newc2py(cname, t, isarray)
    local type = (t[2] and t[1] .. t[2] or t[1]) .. (isarray and '*' or '')
    local ctype = typemap[type]
    if not ctype and (t[2] == '*' or (not t[2] and isarray)) then
        ctype = typemap[type:sub(1,-2)]
    end

    if not ctype then
        error("unknown type " .. type .. ' for ' .. cname)
    end

    typemap[cname] = ctype
end

function c2py(t)
    local type = (t[2] and t[1] .. t[2] or t[1])
    local ctype = typemap[type]
    if not ctype then
        error("unknown type " .. type)
    end
    imported[ctype] = true
    return ctype
end

defs={
    using=function(t)
        newc2py(t[1], t[2], t[3])
    end,
    typedef=function(t)
        newc2py(t[2], t[1])
    end,
    declareclass=function(name)
        typemap[name..'*'] = typemap['void*']
    end,
    doc=function(s)
        lines[#lines+1] = '# ' .. s
    end,
    multidoc=function(s)
        lines[#lines+1] = '# ' .. s:gsub('\n ?', '\n# ')
    end,
    newclass=function(t)
        local name = t[1]
        local ctype = 'CType_' .. name
        typemap[name..'*'] = prefix_ctypes .. 'POINTER(' .. ctype .. ')'
        lines[#lines+1] = 'class ' .. ctype .. '(ctypes.Structure):\n    _fields_ = ['
        for k,mem in ipairs(t[2]) do
            lines[#lines+1] = '        ("' .. mem[2] .. '", ' .. c2py(mem[1]) .. '),'
        end
        lines[#lines+1] = '    ]\n'
        imported['POINTER'] = true
    end,
    func=function(t)
        local types = {}
        for _k,p in ipairs(t[3]) do
            local c_type = c2py(p)
            types[#types+1] = c_type
        end
        lines[#lines+1] = prefix .. t[2] .. '.argtypes = [' .. table.concat(types, ', ') .. ']'
        lines[#lines+1] = prefix .. t[2] .. '.restype = ' .. c2py(t[1])
        lines[#lines+1] = ''
    end,
    strfunc=function(s)
        lines[#lines+1] = '# ' .. s:gsub('\n *', '\n#     ')
        gfunc:match(s)
    end,
}

-- http://www.inf.puc-rio.br/~roberto/lpeg/re.html

local pcommun = [=[

id          <- [_a-zA-Z0-9]+
type        <- {| ('const' ws)? {'unsigned '? id ' long'?} (ws 'const')? S {'*'?} S |}

ws          <- %s+
S           <- %s*

]=]

local pfunc = [=[

function    <- {| type {id} '(' S params S ')' S ';' |} -> func
params      <- {| (param (',' S)?)* |}
param       <- S type S id S

]=] .. pcommun

local p = [=[

start       <- (!extern .)* extern S pattern
extern      <- 'extern' ws '"C"' S '{'
pattern     <- (S (function / using / endexpr))+
function    <- "REDEMPTION_LIB_EXPORT" ws { [^;]+ ';' } -> strfunc
using       <- "using " {| {id} S '=' S type S ({ '[' [^]]* ']' })? |} -> using S ';'
typedef     <- "typedef " {| type ws {id} |} -> typedef S ';'
endexpr     <- comment1 / comment2 / forward / class
comment1    <- '//' '/'? ' '? { [^%nl]* } -> doc %nl
comment2    <- '/*' { (!'*/' .)* } -> multidoc '*/'
forward     <- ('class' / 'struct') ws { id } -> declareclass S ';'
class       <- {| 'struct' ws { id } S '{' memvars S '}' S ';' |} -> newclass
memvars     <- {| ( {| S type S {id} S ';' |} )* |}

]=] .. pcommun

if not arg[1] then
    io.stderr:write('Usage:\n' .. arg[0] .. ' <file.h> [prefix_lib] [prefix_ctypes] {type=ctype}...\n')
    return 1
end

prefix = arg[2] or ''
if arg[3] then
    prefix_ctypes = arg[3]
    for k,v in pairs(typemap) do
        typemap[k] = prefix_ctypes .. v
    end
end
for i=4,#arg do
    local type, ctype = arg[i]:match('([^=]+)=(.*)')
    typemap[type] = ctype
end

f,e=io.open(arg[1])
if e then
    error(e)
end
g=re.compile(p,defs)
gfunc=re.compile(pfunc,defs)

if not g:match(f:read('*a')) then
    error('parsing error')
end

print("# ./tools/cpp2ctypes.lua '" .. table.concat(arg, "' '") .. "'\n")
if #prefix_ctypes == 0 then
    local t = {}
    for k,_ in pairs(imported) do
        if basetype[k] then
            t[#t+1] = k
        end
    end
    table.sort(t)
    print('from ctypes import ' .. table.concat(t, ', '))
    print()
end
print(table.concat(lines, '\n'))
