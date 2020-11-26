#!/usr/bin/env sh

set -e

cd "$(dirname "$0")/.."
d="$(bjam toolset=clang "$@" cwd | sed '/^CWD/!d;s/^CWD: //')"
echo "cwd: $d"
cd "$d"

port=${PORT:=7543}
python3 -m http.server $port --bind 127.0.0.1&

sleep .5
${BROWSER:=xdg-open} "http://localhost:$port/client.html?ws=${WS}&user=${USER}&pass=${PASSWORD}"

fg
