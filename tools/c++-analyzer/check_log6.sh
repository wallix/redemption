#!/usr/bin/env sh

d=$(realpath -m "$0"/..)
cd "$d/../.."
# tools/check_log6.lua src/**/*.{h,c}pp
find src \( -name '*.hpp' -or -name '*.cpp' \) -exec "$d"/check_log6.lua '{}' '+'
