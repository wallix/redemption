#!/bin/bash

cd "$(dirname $0)"/..

build_dir=${TMPDIR:-/tmp/}/coverage-lcov.bin
out_base=${TMPDIR:-/tmp/}/lcov.html

sed_path_project="${PWD//\//\\\/}"

cov_clean_name=coverage-clean.info

#rm -rf "$build_dir"

# bjam --toolset=gcc --build-dir="$build_dir" clean
targets="$@"
[ -z "$targets" ] && targets=$(sed -n -E '/^ *test-canonical /{s#^.+/([^.]+)\.h.*#test_\1#p}' Jamroot)
bjam \
    --toolset=gcc \
    --build-dir="$build_dir" \
    linkflags=--coverage \
    cxxflags='--coverage --test-coverage --profile-arcs' \
    $targets

dirbase=$(echo "$PWD/$build_dir"/*/*/release)
dirbase_len=$((${#dirbase}+7)) # + /tests/
find "$dirbase" -name 'test_*.gcda' | while read f ; do
    base="${f%/*}"
    target="${f##*/}"
    target="${target/\.gcda}"
    gcno="${f/\.gcda}.gcno"
    mv "$dirbase/$target" "$base" 2>/dev/null || continue
    "$base/$target"
    out="$out_base"/"$target"
    covfile="$out"/coverage.info
    cleaned_covfile="$out/$cov_clean_name"
    f="${f:$dirbase_len}"
    base="${f%/*}"
    esc_base="${base//\//\\/}"

    mkdir -p "$out"/full "$out"/cleaned &&
    lcov -c -d "$gcno" -o "$covfile" -b "$PWD" &&
    sed -i '/^SF:\(\/usr\/\|'"$sed_path_project\/tests\/$esc_base\/$target"'\..pp$\)/,/end_of_record/d' "$covfile" &&
    sed '/^SF:'"$sed_path_project\/src\/$esc_base\/${target:5}\..pp"'$/,/end_of_record/p;d' "$covfile" > "$cleaned_covfile" &&
    lcov -l "$covfile" &&
    genhtml -o "$out"/full --demangle-cpp -t "$target" "$covfile" &&
    genhtml -o "$out"/cleaned --demangle-cpp -t "$target" "$cleaned_covfile" &&
    sed -i '/coverFile/s#href="[^"]\+">[^<]\+#href="'"$f"'.gcov.html">'"$f#" "$out"/cleaned/index.html || exit $?
done

exit

find "$out_base" -name "$cov_clean_name" -exec cat '{}' + > "$out_base"/coverage.info &&
genhtml -o "$out_base"/all_tests --demangle-cpp -t 'all tests' "$out_base"/coverage.info || exit $?
