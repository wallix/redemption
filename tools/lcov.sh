#!/bin/bash

cd "$(dirname $0)"/..

build_dir=coverage-lcov.bin
out_base=lcov.html

cd ../..
sed_path_project="${PWD//\//\\\/}"
cd - >/dev/null

cov_clean_name=coverage-clean.info

#rm -rf "$build_dir"

bjam --toolset=gcc --build-dir="$build_dir" clean
grep ^test-canonical Jamroot | while read x f o ; do
    base="${f%/*}"
    target="${f##*/}"
    target=test_"${target%\.hpp}"
    out="$out_base"/"$f"
    covfile="$out"/coverage.info
    cleaned_covfile="$out/$cov_clean_name"
    bjam --toolset=gcc --build-dir="$build_dir" linkflags=-coverage cxxflags=-coverage "$target" \
    | grep \\.run$ | while read x d ; do
        mkdir -p "$out"/full "$out"/cleaned &&
        lcov -c -d "${d/\.run}.gcno" -o "$covfile" -b "$PWD" &&
        sed -i '/^SF:\(\/usr\/\|'"$sed_path_project\/tests\/${base//\//\\/}\/$target"'\.cpp$\)/,/end_of_record/d' "$covfile" &&
        sed '/^SF:'"$sed_path_project\/src\/${f//\//\\/}"'$/,/end_of_record/p;d' "$covfile" > "$cleaned_covfile" &&
        lcov -l "$covfile" &&
        genhtml -o "$out"/full --demangle-cpp -t "$target" "$covfile" &&
        genhtml -o "$out"/cleaned --demangle-cpp -t "$target" "$cleaned_covfile" &&
        sed -i '/coverFile/s#href="[^"]\+">[^<]\+#href="'"$f"'.gcov.html">'"$f#" "$out"/cleaned/index.html || exit $?
    done
done

find "$out_base" -name "$cov_clean_name" -exec cat '{}' + > "$out_base"/coverage.info &&
genhtml -o "$out_base"/all_tests --demangle-cpp -t 'all tests' "$out_base"/coverage.info || exit $?
