#!/usr/bin/env bash

set -o errexit -o pipefail -o nounset

cd $(dirname "$0")/../projects/redemption_configs/

function generate_html()
{
    tools/spec2html.py "$1"
}

function get_progress()
{
    grep -oE '[0-9]+ / [0-9]+' "$1"
}

generate_html autogen/spec/rdp.spec > rdp_cps.html
generate_html <(
    sed -E 's/\)gen_config_ini"( << )?|( << )?R"gen_config_ini\(|#include "config_variant.hpp"//g' \
        autogen/include/configs/autogen/str_python_spec.hpp
) > rdp_ops.html
read OPT_NUM sep OPT_TOTAL < <(get_progress rdp_ops.html)
read CP_NUM sep CP_TOTAL   < <(get_progress rdp_cps.html)

echo "Documented parameters $(($OPT_NUM + $CP_NUM)) / $(($OPT_TOTAL + $CP_TOTAL))"
result=$((($OPT_NUM + $CP_NUM) * 10000 / ($OPT_TOTAL + $CP_TOTAL)))
echo "Percentage done $((result / 100)).$((result % 100)) %"
