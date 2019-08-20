#!lua

function readall(fname)
    f,e = io.open(fname)
    if e then
        error(e)
    end
    local s = f:read('*a')
    f:close()
    return s
end

local log6, NewLinePos

do
    local lpeg = require'lpeg'

    local R = lpeg.R
    local C = lpeg.C
    local P = lpeg.P
    local S = lpeg.S
    local V = lpeg.V
    local Ct = lpeg.Ct
    local Cp = lpeg.Cp()

    function Until(p, strip)
        p = P(p)
        return ((1-(strip or p))^0 * p)
    end

    local logid = P'LogId::'
    local kvlog = P'KVLog::'
    local WithinBalanced = ((1 - S'()') + V'Balanced')^0 * ')'
    local Balanced = '(' * WithinBalanced
    local LogId = logid * C((R('az','AZ','09') + '_')^1)
    local KVArgs = kvlog * Ct(C(R('az','AZ','09')^1) * '("' * C((1-P'"')^1)) * WithinBalanced

    Log6 = P{
        Ct(Ct( Until('log6') * Cp * '('
            * Ct(Until(LogId, logid+kvlog)^0)
            * Ct(Until(KVArgs, kvlog+';')^0)
            )^0),
        Balanced = Balanced,
    }

    NewLinePos = Ct(Until('\n' * Cp)^0 * Cp)
end

function lower_bound(t, value, ibegin, iend)
    local count, i = iend-ibegin
    while count > 0 do
        step = math.floor(count / 2)
        i = ibegin + step
        if t[i] < value then
            i = i + 1
            ibegin = i
            count = count - step + 1
        else
            count = step
        end
    end
    return ibegin
end

local show_log = false
if arg[1] == '-v' then
    show_log = true
    table.remove(arg, 1)
end


logs = {}
for _,fname in ipairs(arg) do
    filecontent = readall(fname)
    r = Log6:match(filecontent)
    if r then
        poslines = NewLinePos:match(filecontent)

        local iline = 1
        for _,t in pairs(r) do
            st = {}
            for _,kvlog in pairs(t[3]) do
                st[#st+1] = kvlog[1]
                st[#st+1] = kvlog[2]
            end
            iline = lower_bound(poslines, t[1], iline, #poslines+1)
            strkvlog = table.concat(st, ' ')
            for _,id in pairs(t[2]) do
                logs[#logs+1] = {fname, iline, id, strkvlog}
            end
        end
    end
end

if #logs == 0 then
    print('log6(...) not found')
    print('Usage: check_log6.lua [-v] src/**/*.{h,c}pp')
    return 1
end

table.sort(logs, function(a, b) return a[3] < b[3] end)

function printlog(info)
    print(string.format('%s:%d:\n  %s  %s', info[1], info[2], info[3], info[4]))
end

if show_log then
    for _,v in ipairs(logs) do
        printlog(v)
    end
end

previouslog = logs[1]
errcode = 0
for i=2,#logs do
    log = logs[i]
    if previouslog[3] == log[3] and previouslog[4] ~= log[4] then
        print('log6 differ:')
        printlog(previouslog)
        printlog(logs[i])
        errcode = errcode + 1
    end
    previouslog = log
end

os.exit(math.min(errcode, 255))
