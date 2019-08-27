cd "$(basename "$0")/.."
# tools/check_log6.lua src/**/*.{h,c}pp
find src \( -name '*.hpp' -or -name '*.cpp' \) -exec lua tools/check_log6.lua '{}' '+'
