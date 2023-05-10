#!/usr/bin/env bash

# cd /var/lib/jenkins/jobs/redemption-future/workspace
cd $(realpath -m "$0/../..")

set -ex

free

typeset -i fast=1
typeset -i update=0
for act in "$@" ; do
    case "$act" in
        full) fast=0 ;;
        update) update=1 ;;
        *)  set +x ; echo "$0 [full] [update]

fast: enable fast compilation
update: update targets.jam and others generated files";
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


# lua analyzer
./tools/c++-analyzer/redemption-analyzer.sh
show_duration redemption-analyzer.sh


# Python tests
# @{
python_test()
{
    pushd "$1"
    python3 -m unittest discover -t . tests
    popd
}

python_test tools/sesman
python_test tools/conf_migration_tool

show_duration "python tests"
# @}


# Lua tests
# @{
eval "$(luarocks path)"

./tools/cpp2ctypes/test.sh

show_duration "lua tests"
# @}



# BJAM Build Test
echo -e "
using gcc : : g++ -DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING ;
using gcc : 8.0 : g++-8 -DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING ;
using clang : 13 : clang++-13 -DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING -Wno-reserved-identifier ;
" > project-config.jam
valgrind_compiler=gcc-8
toolset_gcc=toolset=gcc
toolset_wab=toolset=gcc-8
gcovbin=gcov
toolset_clang=toolset=clang

export REDEMPTION_TEST_DO_NOT_SAVE_IMAGES=1
export LSAN_OPTIONS=exitcode=0 # re-trace by valgrind
export UBSAN_OPTIONS=print_stacktrace=1

export BOOST_TEST_COLOR_OUTPUT=0

# TODO
# if (( $fast == 0 )); then
#     rm -rf bin
# else
#     rm -rf bin/tmp/
# fi

mkdir -p bin/tmp
export TMPDIR_TEST=bin/tmp/

# export REDEMPTION_LOG_PRINT=1
export REDEMPTION_LOG_PRINT=0
export cxx_color=never

export BOOST_TEST_RANDOM=$RANDOM
echo random seed = $BOOST_TEST_RANDOM

build()
{
    bjam -q "$@" || {
        local e=$?
        export REDEMPTION_LOG_PRINT=1
        bjam -q "$@" -j1
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
# build $toolset_wab cxxflags=-g
# multi-thread
build $toolset_wab -j4 cxxflags=-g

show_duration $toolset_wab

# Warn new files created by tests.
set -o pipefail
diff <(echo "$beforerun") <(rootlist) | while read l ; do
    echo "Jenkins:${diffline:-0}:0: warnings: $l [directory integrity]"
    ((++diffline))
done || echo "Directory integrity error: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
set +o pipefail


if (( $fast == 0 )); then
    # valgrind
    #find ./bin/$gcc/release/tests/ -type d -exec \
    #  ./tools/c++-analyzer/valgrind -qd '{}' \;
    /usr/bin/time --format="%Es - %MK" \
      find ./bin/$valgrind_compiler/release/tests/ -type d -exec \
      parallel -j2 ./tools/c++-analyzer/valgrind -qd ::: '{}' +

    show_duration valgrind

    build $toolset_clang -j2 -sNO_FFMPEG=1 san # -s FAST_CHECK=1
    rm -rf bin/clang*

    show_duration $toolset_clang


    # debug with coverage
    build $toolset_gcc -j4 debug -s FAST_CHECK=1 cxxflags=--coverage linkflags=-lgcov
    while read -a a ; do
        echo "gcov: lines: ${a[1]}  exec: ${a[2]}  cover: ${a[3]}"
        declare -i cover=${a[3]:0:-1} i=0
        # 1% = 1 line for graph extractor
        for ((i=0; i<$cover; ++i)); do
            echo 'coverage: '$i
        done
        break
    done < <(gcovr --gcov-executable $gcovbin -r . -f src/ bin/gcc*/debug/ | tail -n2)
    rm -r bin/gcc*

    show_duration $toolset_gcc debug

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
    CLANG_TIDY=clang-tidy-13 \
      ./tools/c++-analyzer/clang-tidy | sed -E '/^(.+\/|)modules\//,/\^/d'

    show_duration clang-tidy
fi
