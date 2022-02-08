#!/usr/bin/env lua

if not pcall(require, 're') then
    io.stderr:write([[Module not found.

Install re module with

    luarocks install --local lpeg

Or

    luarocks --lua-version=5.1 install --local lpeg

Then configure with

    eval "$(luarocks path)
]])
    os.exit(1)
end
re = require're'

typemap = {
    ['void*']='c_void_p',
    ['char*']='c_char_p',
    ['char']='c_char',
}

pytypemap = {
    ['char*']='bytes',
    ['void']='None',
}

basetypes = {POINTER='POINTER', CFUNCTYPE='CFUNCTYPE'}

do
    for _,v in pairs(typemap) do
        basetypes[v] = v
    end
    typemap['void'] = 'None'

    for ctype,pytype in pairs({
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
        ['size_t']='c_size_t',
        ['ssize_t']='c_ssize_t',
        ['float']='c_float',
        ['double']='c_double',
        ['long double']='c_longdouble',
    }) do
        pointer = 'POINTER(' .. pytype .. ')'
        pytypemap[ctype] = 'int'
        typemap[ctype] = pytype
        typemap[ctype .. '*'] = pointer
        basetypes[pytype] = pytype
        basetypes[pointer] = 'POINTER'
    end

    pytypemap['float'] = 'float'
    pytypemap['double'] = 'float'
    pytypemap['long double'] = 'float'
end

imported = {}
lines = {}
enum_lines = {}
prefix = ''
prefix_ctypes = ''

function normalize_type(t)
    return t[2] and t[1] .. t[2] or t[1]
end

-- t = { type:string, optional['*'] }
function to_ctype(cname, t, isarray)
    local type = normalize_type(t) .. (isarray and '*' or '')
    local ctype = typemap[type]
    if not ctype and (t[2] == '*' or (not t[2] and isarray)) then
        ctype = typemap[type:sub(1,-2)]
    end

    if not ctype then
        error("Unknown type " .. type .. ' for ' .. cname)
    end

    return ctype
end

-- t = { type:string, optional['*'] }
function newc2py(cname, t, isarray)
    typemap[cname] = to_ctype(cname, t, isarray)
end

function c2py(t)
    local type = normalize_type(t)
    local ctype = typemap[type]
    if not ctype then
        io.stderr:write("Unknown type '" .. type ..
                        "'\nPlease, run with '" .. type .. "=MyPythonType'\n")
        os.exit(3)
    end
    imported[ctype] = true
    return ctype
end

local defs = {
    using=function(t)
        -- is a function pointer
        if t[4] then
            name = t[1]
            types = {to_ctype(name, t[2])} -- result type
            for _,x in ipairs(t[4]) do
                types[#types+1] = to_ctype(name, x) -- param type
            end
            typemap[name..'*'] = name
            imported['CFUNCTYPE'] = true
            lines[#lines+1] = name .. ' = CFUNCTYPE(' .. table.concat(types, ', ') .. ')\n'
        else
            newc2py(t[1], t[2], t[3] ~= '')
        end
    end,
    typedef=function(t)
        newc2py(t[2], t[1])
    end,
    declareclass=function(name)
        typemap[name..'*'] = typemap['void*']
        pytypemap[name..'*'] = name
    end,
    declareenum=function(t)
        if t[4] then
            typemap[t[2]] = typemap[t[4]]
            pytypemap[t[2]] = 'int'
            t[4] = ': ' .. t[4]
        end
        local s = table.concat(t)
        local content = genumvalues:match(t[5])
        if content then
            enum_lines[#enum_lines+1] = 'class ' .. t[2] .. '(Enum):'
            enum_lines[#enum_lines+1] = content
            enum_lines[#enum_lines+1] = '\n'
        end
        lines[#lines+1] = '# ' .. s:gsub('\n ?', '\n# ')
    end,
    doc=function(s)
        if s == '@}' and lines[#lines] == '' then
            lines[#lines] = '# ' .. s
            lines[#lines+1] = ''
        else
            lines[#lines+1] = '# ' .. s
        end
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
            types[#types+1] = c2py(p[1])
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

local enumdefs = {
    values=function(t)
        local xs = {}
        local previous_value = -1
        local values = {}
        local num
        for _,p in ipairs(t) do
            if p[2] then
                num = p[2]:gsub("'",'')
                previous_value = tonumber(num) or xs[p[2]]
            else
                previous_value = previous_value + 1
            end
            xs[p[1]] = previous_value
            values[#values+1] = '    ' .. p[1] .. ' = ' .. previous_value
        end
        return table.concat(values, '\n')
    end,
}

function string.endswith(self, pattern)
    return self:sub(#self-#pattern+1) == pattern
end

local functions_by_classes = {}
local classes = {}

local classdefs = {
    func=function(t)
        local type = nil
        local iparam = 1
        local special_name = nil

        local funcname = t[2]
        local cparams = t[3]

        if funcname:endswith('_new') then
            type = t[1]
            special_name = '__init__'
        -- skip function without parameter
        elseif #cparams == 0 then
            return
        else
            type = cparams[1][1]
            -- skip first parameter
            iparam = 2
            if funcname:endswith('_delete') then
                special_name = '__del__'
            end
        end

        type = normalize_type(type)
        -- skip function without user type
        if typemap[type] ~= 'c_void_p' then
            return
        end
        type = type:sub(0, #type-1)

        local params = {}
        for i=iparam,#cparams do
            params[#params+1] = {normalize_type(cparams[i][1]), cparams[i][2]}
        end

        local func = {funcname, normalize_type(t[1]), params, special_name}
        local functions = functions_by_classes[type]
        if not functions then
            classes[#classes+1] = type
            functions_by_classes[type] = {func}
        else
            functions[#functions+1] = func
        end
    end,
}

-- http://www.inf.puc-rio.br/~roberto/lpeg/re.html

local penumvalues = [=[

start       <- S '{' {| (ws / comment / {| {id} S ('=' S {id} S)? |} ','?)+ |} -> values

id          <- [_a-zA-Z0-9']+
comment     <- comment1 / comment2
comment1    <- '//' '/'? ' '? [^%nl]* %nl
comment2    <- '/*' (!'*/' .)* '*/'

ws          <- %s+
S           <- %s*

]=]

local pcommun = [=[

id          <- [_a-zA-Z0-9]+
type        <- {| ('const' ws)? 'std::'? {native_type / id} (ws 'const')? S {'*'?} S |}
scalar_w1   <- 'char' / 'short' / 'int' / 'long' / 'double'
native_type <- 'unsigned ' scalar_w1 ' long'? / 'long '+ 'double'

ws          <- %s+
S           <- %s*

]=]

local pfunc = [=[

function    <- {| type {id} '(' S params S ')' S ('noexcept' S)? ';' |} -> func
params      <- {| (param (',' S)?)* |}
param       <- {| S type S {id} S |}

]=] .. pcommun

local p = [=[

start       <- (!extern .)* extern S pattern
extern      <- 'extern' ws '"C"' S '{'
pattern     <- (S (function / using / endexpr))+
function    <- "REDEMPTION_LIB_EXPORT" ws { [^;]+ ';' } -> strfunc
using       <- "using " {| {id} S '=' S type S {( '[' [^]]* ']' )?} S using_fparams? |} -> using ';'
using_fparams <- '(' S {| using_params |} S ')' S ('noexcept' S)? S
using_params  <- (using_param (',' S)?)*
using_param   <- S type S (id S)?
typedef     <- "typedef " {| type ws {id} |} -> typedef S ';'
endexpr     <- comment / forward / class / enum
comment     <- comment1 / comment2
comment1    <- '//' '/'? ' '? { [^%nl]* } -> doc %nl
comment2    <- '/*' { (!'*/' .)* } -> multidoc '*/'
forward     <- ('class' / 'struct') ws { id } -> declareclass S ';'
class       <- {| 'struct' ws { id } S '{' memvars S '}' S ';' |} -> newclass
enum        <- {| { 'enum' S ('class' S)? ( '[[' [^]]+ ']]' S )? } {id} {[^;:]*} (':' S {id} {[^;]*} )? ';' %s? |} -> declareenum
memvars     <- {| ( {| S (comment S)* type S {id} S ';' |} )* |}

]=] .. pcommun

function usage()
    io.stderr:write('Usage:\n' .. arg[0] .. ' [-c] <file.h> [prefix_lib] [prefix_ctypes] {type=ctype}...\n\n -c generate python class\n')
end

local gen_class = false
local filecontents = ''

-- parse cli
do
    local iargc = 1

    -- -c flag
    if arg[iargc] == '-c' then
        gen_class = true
        iargc = iargc + 1
    end

    -- read file
    local filename = arg[iargc]
    if not filename then
        usage()
        return 1
    end
    local file,e = io.open(arg[iargc])
    if e then
        usage()
        error(e)
    end
    filecontents = file:read('*a')
    file:close()

    -- prefix_lib
    prefix = arg[iargc+1] or ''

    -- prefix_ctypes
    if arg[iargc+2] then
        prefix_ctypes = arg[iargc+2]
        for k,v in pairs(typemap) do
            typemap[k] = prefix_ctypes .. v
        end
    end

    -- add ctypes types
    for i=iargc+3,#arg do
        local type, ctype = arg[i]:match('([^=]+)=(.*)')
        typemap[type] = ctype
    end
end

g = re.compile(p,defs)
gfunc = re.compile(pfunc, gen_class and classdefs or defs)
genumvalues = gen_class and re.compile(penumvalues, enumdefs) or {match=function(...) end}

if not g:match(filecontents) then
    error('parsing error')
end

print("# ./tools/cpp2ctypes/cpp2ctypes.lua '" .. table.concat(arg, "' '") .. "'\n")

if not gen_class then
    if #prefix_ctypes == 0 then
        local kctypes, ctype = {}
        for k,_ in pairs(imported) do
            ctype = basetypes[k]
            if ctype then
                kctypes[ctype] = true
            end
        end

        local ctypes = {}
        for k,_ in pairs(kctypes) do
            ctypes[#ctypes+1] = k
        end
        table.sort(ctypes)

        print('from ctypes import ' .. table.concat(ctypes, ', '))
        print()
    end

    print(table.concat(lines, '\n'))
else
    local common_prefix = function(s1, s2, maxlen)
        for i=1,maxlen do
            if s1:byte(i) ~= s2:byte(i) then
                return i-1
            end
        end
        return maxlen
    end

    strings = {}

    -- enums
    if #enum_lines ~= 0 then
        strings[#strings+1] = 'from enum import Enum\n\n'
        strings[#strings+1] = table.concat(enum_lines, '\n')
    end

    for _,classname in ipairs(classes) do
        strings[#strings+1] = 'class ' .. classname .. ":\n    __slot__ = ('_ctx')\n\n"

        local functions = functions_by_classes[classname]
        local funcname = functions[1][1]
        local commonlen = #funcname
        for i=2,#functions do
            commonlen = common_prefix(funcname, functions[i][1], commonlen)
        end

        for _,func in ipairs(functions) do
            strings[#strings+1] = '    def ' .. (func[4] or func[1]:sub(commonlen+1)) .. '(self'

            -- func parameters
            for _,param in ipairs(func[3]) do
                strings[#strings+1] = ', ' .. param[2] .. ':' .. (pytypemap[param[1]] or '')
            end

            -- result type
            local resulttype = func[4] and 'None' or pytypemap[func[2]] or ''
            strings[#strings+1] = ') -> ' .. resulttype .. ':\n        '

            local isinit = (func[4] == '__init__')

            -- prefix line
            if resulttype ~= 'None' then
                strings[#strings+1] = 'return '
            elseif isinit then
                strings[#strings+1] = 'self._ctx = '
            end

            -- call lib
            strings[#strings+1] = 'lib.' .. prefix .. func[1] .. '('
            local params = {}
            if not isinit then
                params[1] = 'self._ctx'
            end
            for _,param in ipairs(func[3]) do
                params[#params+1] = param[2]
            end
            strings[#strings+1] = table.concat(params, ', ')
            strings[#strings+1] = ')'

            -- check context for __init__
            if isinit then
                strings[#strings+1] = '\n        if not self._ctx:'
                strings[#strings+1] = '\n            raise Exception("malloc error")'
            end

            strings[#strings+1] = '\n\n'
        end
    end

    print(table.concat(strings))
end
