#!/usr/bin/env sh

set -e

cd "$(dirname "$0")/.."
d="$(bjam toolset=clang "$@" cwd | sed '/^CWD/!d;s/^CWD: //')"
echo "cwd: $d"
cd "$d"

port=${PORT:=7543}
python -m SimpleHTTPServer $port&

sleep .5
${BROWSER:=xdg-open} "http://localhost:$port/client.html?ws=${WS}&user=${USER}&pass=${PASSWORD}"

fg
