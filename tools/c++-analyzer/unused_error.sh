#!/usr/bin/env bash

d=$(realpath -m "$0"/..)
cd "$d/../.."

out=$(comm -23 <(
    sed -nE 's/.*\((ERR_[_A-Z0-9]+)\).*/\1/p' src/core/error.hpp | sort -u
) <(
    find src \( \( -name '*.hpp' -or -name '*.cpp' \) -and -not -name error.hpp \) -exec \
        sed -nE 's/.*(ERR_[_A-Z0-9]+).*/\1/p' '{}' '+' \
    | sort -u
))

if [[ $out != "ERR_CREDSSP_NTLM_INIT_FAILED" ]]; then
    echo "$out"
    exit 1
fi
