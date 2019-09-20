#!/usr/bin/env bash

d=$(realpath -m "$0"/..)
cd "$d/../.."

out=$(comm -3 <(
    grep -Eo 'cfg:[:_a-zA-Z0-9]+' \
        projects/redemption_configs/autogen/include/configs/autogen/variables_configuration.hpp \
    | sort -u
) <({
    echo 'cfg::icap_server_down::enable_x_context'
    echo 'cfg::icap_server_down::host'
    echo 'cfg::icap_server_down::port'
    echo 'cfg::icap_server_down::service_name'
    echo 'cfg::icap_server_down::tls'
    echo 'cfg::icap_server_up::enable_x_context'
    echo 'cfg::icap_server_up::host'
    echo 'cfg::icap_server_up::port'
    echo 'cfg::icap_server_up::service_name'
    echo 'cfg::icap_server_up::tls'
    find src \( -name '*.hpp' -or -name '*.cpp' \) -exec \
        sed -nE 's/.*<(cfg:[:_a-zA-Z0-9]+)>.*/\1/gp' \
        -- projects/redemption_configs/redemption_src/configs/config.cpp \
        '{}' '+'
} | sort -u))

if [[ ! -z $out ]]; then
    echo "$out"
    exit 1
fi
