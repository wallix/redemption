#!/usr/bin/env bash
set -eu
set -o pipefail

if [[ "${DEBUG:-0}" = '1' ]]; then
    set -x
fi

dest=${DEST:-redemption_source}

if (( $# == 0 )) || [[ "$1" = '-h' || "$1" = '--help' ]]; then
    echo "Copy .cpp and .hpp in DEST from a bjam target

        $0 targets...

    Ex: $0 libscytale

Environment variables:
    DEBUG=${DEBUG:-0}
    DEST=$dest" >&2
    exit
fi

redemption_path="$(realpath "$(dirname "$0")"/..)"
pushd "$redemption_path" >/dev/null

# extract hpp/cpp
typeset -A dirset sources
while read l ; do
    if [[ $l =~ ^[a-zA-Z0-9_/]+'.o: '(.*) ]]; then
        for f in ${BASH_REMATCH[1]}; do
            # ignore duplications
            if [[ ! -v sources[$f] ]]; then
                sources[$f]=1
                dirset[${f%/*}]+="$redemption_path/$f "
            fi
        done
    fi
done < <(bjam "$@" cxxflags='-MM -MF -' -a ||:)

# insert all cpp for log.hpp (the choice depends on the compilation options)
if [[ -v sources['src/utils/log.hpp'] ]]; then
    if [[ ! -v sources['src/utils/log_as_logprint.cpp'] ]]; then
        dirset[src/utils]+="$redemption_path/src/utils/log_as_logprint.cpp "
    fi
    if [[ ! -v sources['src/utils/log_as_syslog.cpp'] ]]; then
        dirset[src/utils]+="$redemption_path/src/utils/log_as_syslog.cpp "
    fi
fi

popd >/dev/null

# rm -r "$dest" ||:

# create new arbo
dirs=()
for dir in "${!dirset[@]}"; do
    dirs+=("$dest/$dir")
done
mkdir -p "${dirs[@]}"

# copy files
for dir in "${!dirset[@]}"; do
    cp ${dirset[$dir]} "$dest/$dir"
done
