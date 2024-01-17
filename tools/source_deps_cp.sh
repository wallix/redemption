#!/usr/bin/env bash
set -eu
set -o pipefail

if [[ "${DEBUG:-0}" = '1' ]]; then
    set -x
fi

dest=redemption_source

if (( $# == 0 )) || [[ "$1" = '-h' || "$1" = '--help' ]]; then
    echo "Copy .cpp and .hpp in '$dest' from a bjam target

        $0 targets...

    Ex: $0 libscytale" 2>&1
    exit
fi

# extract hpp/cpp
typeset -A dirset sources
while read l ; do
    if [[ $l =~ ^[a-zA-Z0-9_/]+'.o: '(.*) ]]; then
        for f in ${BASH_REMATCH[1]}; do
            # check duplicate
            if [[ ! -v sources[$f] ]]; then
                sources[$f]=1
                dirset[${f%/*}]+="$f "
            fi
        done
    fi
done < <(bjam "$@" cxxflags='-MM -MF -' -a ||:)

rm -r $dest

# create new arbo
dirs=("${!dirset[@]}")
for i in "${!dirs[@]}"; do
    dirs[$i]=$dest/${dirs[$i]}
done
mkdir -p ${dirs[@]}

# copy files
for dir in "${!dirset[@]}"; do
    cp ${dirset[$dir]} $dest/$dir
done
