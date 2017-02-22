#!/bin/bash

[ -z "$*" ] && echo $0 'filename.cpp [-DRE_PARSER_POOL_STATE] [g++-options]' >&2 && exit 1

g++ \
-Wall \
-Wextra \
-Wundef \
-Wcast-align \
-Wchar-subscripts \
-Wformat-security \
-Wformat \
-Wformat=2 \
-Werror-implicit-function-declaration \
-Wsequence-point \
-Wreturn-type \
-Wfloat-equal \
-Wpointer-arith \
-Wconversion \
-Wsign-compare \
-Wmissing-declarations \
-Wmissing-noreturn \
-Wmissing-format-attribute \
-Wpacked \
-Wredundant-decls \
-Wdouble-promotion \
-Winit-self \
-Wcast-qual \
-Wold-style-cast \
-Woverloaded-virtual \
-Wnon-virtual-dtor \
-O3 \
-march=native \
-DNDEBUG \
-pedantic \
-I $(dirname $0)/../../.. \
"$@" \
-lpcre
