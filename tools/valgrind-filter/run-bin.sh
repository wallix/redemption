#!/bin/bash

[ -z "$1" ] && echo "Usage: $0 bin" >&2 return 1

valgrind --max-stackframe=3595736 --leak-check=full --show-reachable=yes "$1" \
|& "$(dirname "$0")"/valgrind-filter-ssl \
| grep -C1000 -P '\w+\.\w+:\d+|total heap usage:' --color=always
