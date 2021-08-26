#!/usr/bin/env bash

set -ex

# BJAM_BUILD_DIR=
OUTPUT_DIR=${OUTPUT_DIR:-html_coverage}
mkdir -p "$OUTPUT_DIR"

if [[ ! -z "$BJAM_BUILD_DIR" ]]; then
    opts=--build-dir="$BJAM_BUILD_DIR"
fi

build() {
    bjam -s coverage=on $opts \
        linkflags='--coverage' \
        cxxflags='--coverage -fprofile-arcs -ftest-coverage' \
        "$@"
}

build "$@"
d="$(build "$@" cwd | sed '/^CWD/!d;s/^CWD: //')"

run_gcovr() {
    gcovr  -r . -f src/ --html --html-details -o "$OUTPUT_DIR/index.html" "$d"
}

if [[ ! -z "$GCOV_BIN" ]]; then
    run_gcovr --gcov-executable="$GCOV_BIN"
else
    run_gcovr
fi
