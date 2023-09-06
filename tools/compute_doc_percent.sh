#!/usr/bin/env bash

set -o errexit -o pipefail -o noclobber -o nounset

cd $(dirname "$0")/../projects/redemption_configs/

function show_progress()
{
    tools/spec2html.py "$1" | grep -oE '[0-9]+ / [0-9]+'
}

read OPT_NUM sep OPT_TOTAL < <(show_progress autogen/spec/rdp.spec)
read CP_NUM sep CP_TOTAL   < <(show_progress <(
    sed -E 's/\)gen_config_ini"( << )?|( << )?R"gen_config_ini\(//g' \
        autogen/include/configs/autogen/str_python_spec.hpp
))

echo "Documented parameters $(($OPT_NUM + $CP_NUM)) / $(($OPT_TOTAL + $CP_TOTAL))"
result=$((($OPT_NUM + $CP_NUM) * 10000 / ($OPT_TOTAL + $CP_TOTAL)))
echo "Percentage done $((result / 100)).$((result % 100)) %"
