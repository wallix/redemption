#!/usr/bin/env bash

set -o errexit -o pipefail -o noclobber -o nounset

cd $(dirname "$0")/..

RDP_CP_HTML=rdp_connection_policy.html
RDP_OPT_HTML=rdp_options.html
CONFIG_PROJ_PATH=projects/redemption_configs/
CONVERT_TOOL=spec2html.py

{
    cd projects/redemption_configs/
    bjam -j4

    rm "$RDP_CP_HTML"
    tools/"$CONVERT_TOOL" autogen/spec/rdp.spec > "$RDP_CP_HTML"

    cd -

    bjam -j4 rdpproxy

    rm projects/redemption_configs/"$RDP_OPT_HTML"
    bin/gcc-8.3.0/release/rdpproxy --print-spec | projects/redemption_configs/tools/"$CONVERT_TOOL" - > projects/redemption_configs/"$RDP_OPT_HTML"

    read OPT_NUM sep OPT_TOTAL < <(grep -oE '[0-9]+ / [0-9]+' projects/redemption_configs/"$RDP_OPT_HTML")

    read CP_NUM sep CP_TOTAL < <(grep -oE '[0-9]+ / [0-9]+' projects/redemption_configs/"$RDP_CP_HTML")

} &> /dev/null


echo "Documented parameters $(($OPT_NUM + $CP_NUM)) / $(($OPT_TOTAL + $CP_TOTAL))"
echo "Percentage done $((($OPT_NUM + $CP_NUM) * 100 / ($OPT_TOTAL + $CP_TOTAL))) %"
