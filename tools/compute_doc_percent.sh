#!/usr/bin/env bash

set -o errexit -o pipefail -o nounset

outbase=''
if (($# >= 1)) && [[ -n $1 ]]; then
    outbase="$(realpath "$1")"/
fi

cd "$(dirname "$0")"/../projects/redemption_configs/


function compute_percent()
{
    declare -i result=$(($1 * 10000 / $2))
    REPLY=$((result / 100)).$((result % 100))
}

declare -i accu_num=0 accu_total=0 num total
html_link=''

function generate_html_and_extract_percent()
{
    tools/spec2html.py "$1" > "$outbase$2"
    read num sep total < <(grep -oE '[0-9]+ / [0-9]+' "$outbase$2")

    accu_num+=$num
    accu_total+=$total

    compute_percent $num $total
    html_link+="<li><a href='$2'>$3</a> ($num / $total = $REPLY %)</li>"
}


generate_html_and_extract_percent <(
    sed -E 's/\)gen_config_ini"( << )?|( << )?R"gen_config_ini\(|#include "config_variant.hpp"//g' \
        autogen/include/configs/autogen/str_python_spec.hpp
) rdp_ops.html "RDP Configuration options"

generate_html_and_extract_percent autogen/spec/rdp.spec rdp_cps.html "RDP Connection policy"

compute_percent $accu_num $accu_total
total_percent=$REPLY
echo "Documented parameters $accu_num / $accu_total"
echo "Percentage done $total_percent %"

echo "<!DOCTYPE html>
<html>
<head>
<title>RDP Proxy parameters</title>
<body>
<p>RDP Proxy parameters</p>
<ul>${html_link[@]}</ul>
<p>Total: $accu_num / $accu_total = $total_percent %</p>
</body>
</head>
</html>
" > "$outbase"index.html
