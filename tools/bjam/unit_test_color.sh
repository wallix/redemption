#!/bin/sh

esc="$(echo -n "\x1b[")"
reset=$esc'0m'
err=$esc'1;31m'
cat=$esc'34m'
file=$esc'37m'
code=$esc'31m'
value=$esc'36m'
op=$esc'33m'
name=$esc'35m'
line=$esc'1;36m'
term_mess=$esc'1;31m'
exception=$esc'1;37m'
exec=$esc'34m'
msg=$esc'1m'

sed -uE "$@" -e '
s/^\x1b\[0;39;49m//;tx
/^\x1b/n
:x
/^tests\//{
  s/^(tests\/([^.]|..\/)+\.[ch]pp)\(([0-9]+)\):/'$file'\1'$reset'('$line'\3'$reset'):/;tc
  :c

  s/(fatal )?error:? in "([^"]+)": (critical )?check/'$err'\1error'$reset' in "'$name'\2'$reset'": '$cat'\3check'$reset'/;tm
  :m

  s/(check\x1b\[0m )(.*)( !=|==|<=|>=|>|< )(.*) has failed \[(.*)( != | == | <= | >= | > | < )(.*)\](\]\. )?/\1'$code'\2'$op'\3'$code'\4'$reset' has failed ['$value'\5'$op'\6'$value'\7'$reset']/;t

  s/(check\x1b\[0m )(.*) (has )?failed$/\1'$code'\2'$reset' \3failed/;t

  s/last checkpoint$/'$cat'last checkpoint'$reset'/;t
  s/last checkpoint: "([^"]+)" (test )?entry.?$/'$cat'last checkpoint'$reset': "'$name'\1'$reset'" test entry/;t
}
s/^(src|tests|)(\/[^.]+\.[ch]p?p?:[0-9]+:[0-9]+): ([^:]+): (.*)/'$file'\1\2'$reset': '$err'\3'$reset': '$msg'\4'$reset'/;t
s/^SUMMARY: (.*)/'$err'SUMMARY: '$msg'\1'$reset'/;t
s/^unknown location\(0\): fatal error:? in "([^"]+)": ([a-zA-Z0-9]+::)*([^:]+): (.*)$/'$cat'unknown location(0):'$reset' '$err'fatal error'$reset' in "'$name'\1'$reset'": '$cat'\2\3'$reset': '$op'\4'$reset'/;t
s/^terminate called.*$/'$term_mess'&'$reset'/;t
s/^  what\(\): .*$/'$exception'&'$reset'/;t
s/^     "([^"]+)" /    "'$exec'\1'$reset'" /;t
s/^\*\*\* [0-9]+ failures? .*$/'$err'&'$reset'/;t
s/^ *(#[0-9]+ )(.+ )at (.+)(:[0-9]+)?$/'$err'\1'$name'\2'$reset'at '$file'\3'$line'\4'$reset'/;t
s/^ *(#[0-9]+ )(0x[^ ]+) in (.+)(:[0-9]+)?$/'$err'\1'$reset'\2 in '$cat'\3'$line'\4'$reset'/;t
/Assert/{
  s/([^:]+): ([^:]+:[^:]+): ([^:]+): Assertion `(.*) failed.$/'$file'\2'$reset': '$err'failed'$reset' (\1): '$op'\3'$reset': Assertion `'$code'\4'$reset'`/;t
  s/^.*$/'$err'[[&]]'$reset'/;t
}
'
