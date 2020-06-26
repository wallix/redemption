#!/usr/bin/env bash

color=1

set_color()
{
    case "$1" in
        --color=never) color=0 ;;
        --color=always) color=1 ;;
        --color=auto) [ -t 1 ] || color=0 ;;
        *) return 1
    esac
}

set_color "$1" && shift || set_color --color="${HELP_COLOR:-auto}"

if [ $color -eq 1 ]; then
    reset=$'\x1b''[0m'
    title=$'\x1b''[33m'
    var=$'\x1b''[31m'
    sep=$'\x1b''[32m'
    comm=$'\x1b''[37m'
fi

if [ "$1" = "project" ] || [ -z "$1" ] ; then
    echo
    echo $title'# Project config for `-s varname=value`:'$reset
    echo
    sed -E 's/.*\[ setvar ([^ ]+) : (.*).* \] ;(.*)/'$var'\1 '$sep'='$reset' \2'$comm'\3'$reset'/;t;d' jam/defines.jam
    echo
    echo $title'# Default values are:'$reset
    echo
    sed -E 's/^([A-Z_]+_DEFAULT) [^=]+= (.*) ;/'$var'\1 '$sep'='$reset' \2/;t;d' jam/defines.jam
fi

if [ "$1" = "compiler" ] || [ -z "$1" ] ; then
    echo
    echo $title'# Compiler config for `-s cxx_color=always` or `cxx-color=always`:'$reset
    echo
    sed -E 's/^constant jln_[^[]+\[ jln-get-env ([^ ]+) : ([^]]+) \].*/'$var'\1 '$sep'='$reset' \2/;t;d' jam/cxxflags.jam
fi
