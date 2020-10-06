local io, error, require, pairs, ipairs, string, table = io, error, require, pairs, ipairs, string, table

module "utils"


function println_error(s)
    print_error(s)
    print_error('\n')
end

function print_error(s)
    io.stderr:write(s)
end

function readall(fname)
    f,e = io.open(fname)
    if e then
        error(e)
    end
    local s = f:read('*a')
    f:close()
    return s
end


-- LPeg utility

peg = require'lpeg'

local P = peg.P

peg.Until = function(p)
    return (1-P(p))^0
end

peg.After = function(p)
    p = P(p)
    return ((1-p)^0 * p)
end

peg.wordchars = peg.R('az','AZ','09') + '_'
peg.word = peg.wordchars^1
peg.space = peg.S'\n\t '
peg.ws0 = peg.space^0
peg.ws = peg.space^1
peg.singleLineComment = '//' * peg.After('\n')
peg.multiLineComment = '/*' * peg.After('*/')


-- checker utility

function match_and_setk(pattern, content, value, t)
    local t = t or {}
    local r = pattern:match(content)
    if r then
        for _,v in ipairs(r) do
            t[v] = value
        end
    end
    return t
end

local kvfmt = '%s = %s\n'
local kvnamefmt = '#%s = %d\n'
function print_value(kvalues, name)
    print_error(kvnamefmt:format(name or 'values', #kvalues))
    for name,activated in pairs(kvalues) do
        print_error(kvfmt:format(name, activated))
    end
end

function count_error(kvalues, fmt)
    local t = {}
    for name,activated in pairs(kvalues) do
        if not activated then
            t[#t + 1] = fmt:format(name)
        end
    end
    if #t ~= 0 then
        t[#t+1] = ''
        print_error(table.concat(t,'\n'))
    end
    return #t
end
