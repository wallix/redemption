local utils = require'utils'
local print_error = utils.print_error
local pairs, ipairs, assert, math, table, string = pairs, ipairs, assert, math, table, string

module "log6_checker"

local Log6, LogIds, NewLinePos

do
    local peg = utils.peg
    local C = peg.C
    local P = peg.P
    local S = peg.S
    local V = peg.V
    local Ct = peg.Ct
    local Cp = peg.Cp()
    local After = peg.After
    local Until = peg.Until

    local logid = P'LogId::'
    local idname = C(peg.word)
    local kvlog = P'KVLog'
    local WithinBalanced = ((1 - S'()') + V'Balanced')^0 * ')'
    local Balanced = '(' * WithinBalanced
    local LogId = logid * idname
    local KVArgs = kvlog * '("' * C((1-P'"')^1) * WithinBalanced

    Log6 = P{
        Ct(Ct( After('log6') * Cp * '('
             * Ct((Until(logid+kvlog) * LogId)^0)
             * Ct((Until(kvlog+';') * KVArgs)^0)
             )^0),
        Balanced = Balanced,
    }

    NewLinePos = Ct(After('\n' * Cp)^0 * Cp)

    LogIds = Ct((After(P'  f(') * idname)^0)
end


local lower_bound = function(t, value, ibegin, iend)
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


local ids = {}
local verbose = false

function init(args)
    if args == 'verbose' then
        verbose = true
    end

    local i = 0
    for _,v in ipairs(LogIds:match(utils.readall('src/core/log_id.hpp'))) do
        ids[v] = 0
        i = i + 1
    end
    assert(i > 10) -- random magic number
end

logs = {}
function file(content, filename)
    local r = Log6:match(content)
    if r then
        poslines = NewLinePos:match(content)

        local iline = 1
        for _,t in pairs(r) do
            iline = lower_bound(poslines, t[1], iline, #poslines+1)
            strkvlog = table.concat(t[3], ' ')
            for _,id in pairs(t[2]) do
                logs[#logs+1] = {filename, iline, id, strkvlog}
            end
        end
    end
end

function terminate()
    if #logs == 0 then
        print_error('log6(...) not found\n')
        return 1
    end

    table.sort(logs, function(a, b) return a[3] < b[3] end)

    function printlog(info)
        print_error(string.format('%s:%d:\n  %s  %s\n', info[1], info[2], info[3], info[4]))
    end

    if verbose then
        for _,v in ipairs(logs) do
            printlog(v)
        end
    end

    previouslog = logs[1]
    errcount = 0
    ids[previouslog[3]] = 1
    for i=2,#logs do
        log = logs[i]
        ids[log[3]] = 1
        if previouslog[3] == log[3] and previouslog[4] ~= log[4] then
            print_error('log6 differ:\n')
            printlog(previouslog)
            printlog(logs[i])
            errcount = errcount + 1
        end
        previouslog = log
    end

    -- add not extracted id
    for _,id in ipairs({
        "CERTIFICATE_CHECK_SUCCESS",
        "SERVER_CERTIFICATE_NEW",
        "SERVER_CERTIFICATE_MATCH_SUCCESS",
        "SERVER_CERTIFICATE_MATCH_FAILURE",
        "SERVER_CERTIFICATE_ERROR",
        "PROBE_STATUS",
    }) do
        if ids[id] ~= 0 then
            print_error(id .. ' is already used, please update script\n')
            errcount = errcount + 1
        else
            ids[id] = 1
        end
    end

    for k,v in pairs(ids) do
        if v ~= 1 then
            print_error(k .. ' not used\n')
            errcount = errcount + 1
        end
    end

    return errcount
end
