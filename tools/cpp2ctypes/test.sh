#!/usr/bin/env sh

cd "$(dirname "$0")"

d=${TMPDIR:-/tmp}/cpp2ctypes_
DIFF=${DIFF:-'diff --color=always -u'}
LUA=lua

test_case() {
    $LUA ./cpp2ctypes.lua $2 "./autotest/$1.h" | tail -n+3 > "${d}$1.py"
    if ! cmp "./autotest/$1.py" "${d}$1.py" ; then
        echo $'\e[33mError on '"'$1'"$'\e[0m\n' 2>&1
        $DIFF "./autotest/$1.py" "${d}$1.py"
    fi
}

test_case enums
test_case funcs
test_case pyclass -c
