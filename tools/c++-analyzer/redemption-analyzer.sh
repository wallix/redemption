#!/usr/bin/env sh

# apt install lua luarocks
# luarocks --local install lpeg
# lua >= 5.0

cd "$(realpath -m "$0"/../../..)"
eval `luarocks path`
LUA_PATH="$PWD/tools/c++-analyzer/lua-checker/?.lua;$LUA_PATH"
find src \( -name '*.hpp' -or -name '*.cpp' \) -and -not -path 'src/keyboard/*' -exec \
    ./tools/c++-analyzer/lua-checker/lua-checker.lua --checks \* '{}' '+'
