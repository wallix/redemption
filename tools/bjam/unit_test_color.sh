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

sed -E "$@" -e '/^\x1b/n
/^tests\//{
  s/^(tests\/[^.]+\.[ch]pp)\(([0-9]+)\): error:? in "([^"]+)": check (.*)( !=|==|<=|>=|>|< )(.*) failed \[(.*)( !=|==|<=|>=|>|< )(.*)\](\]\. )?$/'$file'\1'$reset'('$line'\2'$reset'): '$err'error'$reset' in "'$name'\3'$reset'": '$cat'check'$reset' '$code'\4'$op'\5'$code'\6'$reset' failed ['$value'\7'$op'\8'$value'\9'$reset']/;t

  s/^(tests\/[^.]+\.[ch]pp)\(([0-9]+)\): fatal error:? in "([^"]+)": critical check (.*)( !=|==|<=|>=|>|< )(.*) failed \[(.*)( !=|==|<=|>=|>|< )(.*)\](\]\. )?$/'$file'\1'$reset'('$line'\2'$reset'): '$err'fatal error'$reset' in "'$name'\3'$reset'": '$cat'critical check'$reset' '$code'\4'$op'\5'$code'\6'$reset' failed ['$value'\7'$op'\8'$value'\9'$reset']/;t

  s/^(tests\/[^.]+\.[ch]pp)\(([0-9]+)\): (fatal )?error:? in "([^"]+)": (critical )?check (.*) (has )?failed$/'$file'\1'$reset'('$line'\2'$reset'): '$err'\3error'$reset' in "'$name'\4'$reset'": '$cat'\5check'$reset' '$code'\6'$reset' \7failed/;t

  s/^(tests\/[^.]+\.[ch]pp)\(([0-9]+)\): (fatal )?error:? in "([^"]+)": (.*)$/'$file'\1'$reset'('$line'\2'$reset'): '$err'\3error'$reset' in "'$name'\4'$reset'": '$cat'\5'$reset'/;t

  s/^(tests\/[^.]+\.[ch]pp)\(([0-9]+)\): last checkpoint$/'$file'\1'$reset'('$line'\2'$reset'): '$cat'last checkpoint'$reset'/;t
  s/^(tests\/[^.]+\.[ch]pp)\(([0-9]+)\): last checkpoint: "([^"]+)" (test )?entry.?$/'$file'\1'$reset'('$line'\2'$reset'): '$cat'last checkpoint'$reset': "'$name'\3'$reset'" test entry/;t
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
