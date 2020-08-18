#!/bin/bash

# cd /var/lib/jenkins/jobs/redemption-future/workspace
cd $(realpath -m "$0/../..")

if ! [[ $PWD =~ ^/var/lib/jenkins/jobs/* ]]; then
  echo 'Please, run script on Jenkins.'
  exit 1
fi

set -ex

typeset -i fast=0
if [ "$1" = fast ]; then
    fast=1
fi

# Cleaning
#rm -fr cmake_temp

git clean -fd
git submodule update --init

if [ $fast -eq 0 ]; then
    ./tools/c++-analyzer/redemption-analyzer.sh
fi

timestamp=$(date +%s)
show_duration()
{
    local timestamp2=$(date +%s)
    echo duration"[$@]": $((($timestamp2-$timestamp)/60))m $((($timestamp2-$timestamp)%60))s
    timestamp=$timestamp2
}

# jsclient (emscripten)
pushd projects/jsclient
source ~/emsdk/emsdk_env.sh
if [ $fast -eq 0 ]; then
    rm -rf bin
fi
version=$(clang++ --version | sed -E 's/^.*clang version ([0-9]+\.[0-9]+).*/\1/;q')
echo "using clang : $version : clang++ -DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING ;" > project-config.jam
if [ ! -d system_include/boost ]; then
    mkdir -p system_include
    ln -s /usr/include/boost/ system_include
fi
if [ ! -d node_modules ]; then
    ln -s ~/node_jsclient/future/node_modules .
fi
set -o pipefail
bjam -qj2 toolset=clang-$version debug cxxflags=-Wno-shadow-field |& sed '#^/var/lib/jenkins/jobs/redemption-future/workspace/##'
set +o pipefail
popd

show_duration jsclient


#These following packages MUST be installed. See README of redemption project
#aptitude install build-essential bjam boost-build libboost-program-options-dev libboost-test-dev libssl-dev locales cmake

# # use libstdc++-8 with clang-8 because version 9 fails with -D_GLIBCXX_DEBUG (ok with clang-9)
# libstdcxx_compact_version=8
# mkdir -p \
#     libstdc++-compact/include/c++ \
#     libstdc++-compact/lib/gcc/x86_64-unknown-linux-gnu
# ln -s /usr/include/c++/$libstdcxx_compact_version libstdc++-compact/include/c++
# ln -s /usr/lib/gcc/x86_64-linux-gnu/$libstdcxx_compact_version libstdc++-compact/lib/gcc/x86_64-unknown-linux-gnu

# BJAM Build Test
echo -e "using gcc : 9.0 : g++-9 -DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING ;\nusing clang : 9.0 : clang++-9 -DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING ;" > project-config.jam
valgrind_compiler=gcc-9
toolset_gcc=toolset=gcc-9
toolset_clang=toolset=clang-9.0

export REDEMPTION_TEST_DO_NOT_SAVE_IMAGES=1
export LSAN_OPTIONS=exitcode=0 # re-trace by valgrind
export UBSAN_OPTIONS=print_stacktrace=1

export BOOST_TEST_COLOR_OUTPUT=0

if [ $fast -eq 0 ]; then
    rm -rf bin
else
    rm -rf bin/tmp/
fi

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
# build $toolset_gcc cxxflags=-g
# multi-thread
big_mem='exe libs
  tests/capture
  tests/lib
  tests/server
  tests/client_mods
  tests/client_redemption/client_channels
  tests/mod/rdp.norec
  tests/mod/vnc.norec'
build $toolset_gcc cxxflags=-g -j2 ocr_tools
build $toolset_gcc cxxflags=-g $big_mem
build $toolset_gcc cxxflags=-g -j2

show_duration $toolset_gcc


# Warn new files created by tests.
set -o pipefail
diff <(echo "$beforerun") <(rootlist) | while read l ; do
    echo "Jenkins:${diffline:-0}:0: warnings: $l [directory integrity]"
    ((++diffline))
done || if [ $fast -eq 1 ]; then
    # error with fast compilation
    echo "Directory integrity error: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
    exit 1
fi
set +o pipefail


#bjam -a -q toolset=clang-8 -sNO_FFMPEG=1 san
build $toolset_clang -sNO_FFMPEG=1 san -j3 ocr_tools -s FAST_CHECK=1
build $toolset_clang -sNO_FFMPEG=1 san $big_mem -s FAST_CHECK=1
build $toolset_clang -sNO_FFMPEG=1 san -j2 -s FAST_CHECK=1

show_duration $toolset_clang


if [ $fast -eq 0 ]; then
    # debug with coverage
    build $toolset_gcc debug -scoverage=on covbin=gcov-7 -s FAST_CHECK=1

    show_duration $toolset_gcc coverage

    # cppcheck
    # ./tools/c++-analyzer/cppcheck-filtered 2>&1 1>/dev/null


    # extract TODO, BUG, etc
    find \
      src \
      tests \
      projects/ClientQtGraphicAPI/src/ \
      projects/redemption_configs/ \
      \( -name '*.h' -o -name '*.hpp' -o -name '*.cpp' \) \
      -exec ./tools/c++-analyzer/todo_extractor '{}' +

    show_duration todo_extractor

    #set -o pipefail

    # clang analyzer
    CLANG_TIDY=clang-tidy-10 /usr/bin/time --format="%Es - %MK" \
      ./tools/c++-analyzer/clang-tidy | sed -E '/^(.+\/|)modules\//,/\^/d'

    show_duration clang-tidy

    # valgrind
    #find ./bin/$gcc/release/tests/ -type d -exec \
    #  ./tools/c++-analyzer/valgrind -qd '{}' \;
    /usr/bin/time --format="%Es - %MK" \
      find ./bin/$valgrind_compiler/release/tests/ -type d -exec \
      parallel -j2 ./tools/c++-analyzer/valgrind -qd ::: '{}' +

    show_duration valgrind
fi
