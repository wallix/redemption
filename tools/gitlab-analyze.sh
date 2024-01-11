#!/usr/bin/env bash

# cd /var/lib/jenkins/jobs/redemption-future/workspace
cd $(realpath -m "$0/../..")

set -ex

free

typeset -i fast=1
for act in "$@" ; do
    case "$act" in
        full) fast=0 ;;
        *)  set +x ; echo "$0 [full]

full: compiles with clang and gcc to relase and debug + valgrind + coverage
";
            exit 1 ;;
    esac
done

timestamp=$(date +%s)
show_duration()
{
    local timestamp2=$(date +%s)
    echo duration"[$@]": $((($timestamp2-$timestamp)/60))m $((($timestamp2-$timestamp)%60))s
    timestamp=$timestamp2
}


# Custom analyzer
# @{
# /usr/share/lua/5.1/ is not exported, because ???
export LUA_PATH="$PWD/tools/c++-analyzer/lua-checker/?.lua;/usr/share/lua/5.1/?.lua"
find src \( -name '*.hpp' -or -name '*.cpp' \) -and -not -path 'src/keyboard/*' -exec \
    ./tools/c++-analyzer/lua-checker/lua-checker.lua --checks \* '{}' '+'

show_duration redemption-analyzer.sh
# @}


# Python tests and coverage
# @{
python_test()
{
    pushd "$1"
    coverage run -m pytest tests
    popd
}

python_test tools/sesman
python_test tools/conf_migration_tool
coverage combine tools/*/.coverage
coverage report
coverage xml -o py_coverage.xml

show_duration "python tests"
# @}


# Python analyzer
# @{
ruff --preview tools/*.py tools/sesman/sesmanworker > ruff_report.txt || true

show_duration "python analyzer"
# @}


# Lua tests
# @{
# eval "$(luarocks path)"

./tools/cpp2ctypes/test.sh

show_duration "lua tests"
# @}

# BJAM Build Test
echo -e "
using gcc : : g++ -DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING ;
using clang : : clang++ -DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING -Wno-reserved-identifier ;
" > project-config.jam
toolset_gcc=toolset=gcc
gcovbin=gcov
toolset_clang=toolset=clang

export REDEMPTION_TEST_DO_NOT_SAVE_IMAGES=1
export LSAN_OPTIONS=exitcode=0 # re-trace by valgrind
export UBSAN_OPTIONS=print_stacktrace=1

# export BOOST_TEST_COLOR_OUTPUT=0

mkdir -p bin/tmp
export TMPDIR_TEST=bin/tmp/

# export REDEMPTION_LOG_PRINT=1
export REDEMPTION_LOG_PRINT=0
# export cxx_color=never

export BOOST_TEST_RANDOM=$RANDOM
echo random seed = $BOOST_TEST_RANDOM

run_bjam() {
    /usr/bin/time --format='%Es - %MK' bjam -q cxxflags=-Wno-deprecated-declarations "$@"
}

build()
{
    run_bjam "$@" || {
        local e=$?
        export REDEMPTION_LOG_PRINT=1
        run_bjam "$@" -j1
        exit $e
    }
}

rootlist()
{
    ls -1
}

# implicitly created by bjam
mkdir -p bin
beforerun=$(rootlist)

# release for -Warray-bounds and not assert
# build $toolset_gcc cxxflags=-g
# multi-thread
build $toolset_gcc -j4 cxxflags=-g

show_duration $toolset_gcc

# Warn new files created by tests.
set -o pipefail
diff <(echo "$beforerun") <(rootlist) | while read l ; do
    echo "Jenkins:${diffline:-0}:0: warnings: $l [directory integrity]"
    ((++diffline))
done || echo "Directory integrity error: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
set +o pipefail


if (( $fast == 0 )); then
    # valgrind
    cmds=()
    while read l ; do
        f=${l#*release/tests/}
        f=valgrind_reports/"${f//\//@}".xml
        cmds+=("valgrind --child-silent-after-fork=yes --xml=yes --xml-file=$f $l")
    done < <(find ./bin/gcc*/release/tests/ -executable -type f -name test_'*')
    mkdir -p valgrind_reports
    /usr/bin/time --format="%Es - %MK" parallel -j4 ::: "${cmds[@]}"

    show_duration valgrind


    build $toolset_clang -j4 -sNO_FFMPEG=1 san
    rm -rf bin/clang*

    show_duration $toolset_clang


    # debug with coverage
    build $toolset_gcc -j4 debug -s FAST_CHECK=1 cxxflags=--coverage linkflags=-lgcov
    gcovr --gcov-executable $gcovbin --xml -r . -f src/ bin/gcc*/debug/ > gcovr_report.xml

    show_duration $toolset_gcc coverage


    # cppcheck
    # ./tools/c++-analyzer/cppcheck-filtered 2>&1 1>/dev/null
    # show_duration cppcheck


    # extract TODO, BUG, etc
    find \
      src \
      tests \
      projects/qtclient/src/ \
      projects/redemption_configs/ \
      \( -name '*.h' -o -name '*.hpp' -o -name '*.cpp' \) \
      -exec ./tools/c++-analyzer/todo_extractor '{}' +

    show_duration todo_extractor

    #set -o pipefail

    # clang analyzer
    ./tools/c++-analyzer/clang-tidy | tee clang_tidy_report.txt

    show_duration clang-tidy
fi
