#!/usr/bin/env bash

d=$(realpath -m "$0"/..)
cd "$d/../.."

out=$(comm -23 <(
    sed -nE 's/.*\.enumeration_[^"]+"([^"]+).*/\1/p' \
        projects/redemption_configs/configs_specs/configs/specs/config_type.hpp \
    | sort -u
) <({
    echo OcrLocale ;
    echo ColorDepth ;
    echo KeyboardLogFlagsCP ;
    find src \( -name '*.hpp' -or -name '*.cpp' \) -exec \
        grep -hEo '\b[A-Z][a-zA-Z]+\b' \
        '{}' '+'
} | sort -u))

if [[ ! -z $out ]]; then
    echo "$out"
    exit 1
fi
