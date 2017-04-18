#!/bin/sh

esc="$(echo -n "\x1b[")"
reset=$esc'0m'
err=$esc'1;31m'
cat=
file=$esc'37m'
code=$esc'31m'
value=$esc'36m'
op=$esc'33m'
name=$esc'35m'

sed -E -e '/^tests\//{
  s/^(tests\/[^.]+\.cpp)\([0-9]+\): error in "([^\"]+)": ([^ ]+) (.*)( !=|==|<|<=|>|>= )(.*) failed \[(.*)( !=|==|<|<=|>|>= )(.*)\]$/'$file'\1'$reset': '$err'error'$reset' in  "'$name'\2'$reset'": '$cat'\3'$reset' '$code'\4'$op'\5'$code'\6'$reset' failed ['$value'\7'$op'\8'$value'\9'$reset']/
  s/^(tests\/[^.]+\.cpp)\([0-9]+\): error in "([^\"]+)": ([^ ]+) (.*) failed$/'$file'\1'$reset': '$err'error'$reset' in  "'$name'\2'$reset'": '$cat'\3'$reset' '$code'\4'$reset' failed/
}' "$@"
