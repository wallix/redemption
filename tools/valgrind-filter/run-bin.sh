#!/bin/bash

[ -z "$1" ] && echo "Usage: $0 valgrind-arguments" >&2 return 1

dir="`dirname "$0"`"
valgrind \
    --leak-check=full \
    --track-fds=yes \
    --show-reachable=yes \
    --suppressions="$dir"/valgrind-ignore-ssl-snappy \
    "$@" \
|& "$dir"/valgrind-output-color
