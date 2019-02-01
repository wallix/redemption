#!/bin/bash

set -e

cd "$(dirname "$0")/.."

contents=$(awk 'BEGIN { pre="                     " }
/^#/{
    lvl=length($1)
    a[lvl] += 1
    a[lvl+1] = 0
    title = substr($0, lvl+2)
    link = tolower(title)
    gsub(/ /, "-", link)
    gsub(/[^-_a-zA-Z0-9]/, "", link)
    summary = summary "\n" substr(pre, 0, (lvl-1)*4) a[lvl] ". [" title "](#" link ")"
}
{ s = s "\n" $0 }
END{
    sub(/\n<!-- summary -->\n.*\n<!-- \/summary -->\n/, "\n<!-- summary -->" summary "\n<!-- /summary -->\n", s)
    print substr(s, 2)
}' README.md)

echo -E "$contents" > README.md
