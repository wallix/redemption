#!/usr/bin/env sh

cd "$(realpath -m "$0"/../../..)"
source=`luarocks path` || {
    echo 'apt install lua luarocks
luarocks --local install lpeg argparse
lua >= 5.0'
    exit 1
}
eval "$source"
LUA_PATH="$PWD/tools/c++-analyzer/lua-checker/?.lua;$LUA_PATH"
find src \( -name '*.hpp' -or -name '*.cpp' \) -and -not -path 'src/keyboard/*' -exec \
    ./tools/c++-analyzer/lua-checker/lua-checker.lua --checks \* '{}' '+'
