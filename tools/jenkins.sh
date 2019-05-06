#!/bin/bash

# cd /var/lib/jenkins/jobs/redemption-future/workspace
cd $(realpath -m "$0/../..")

if ! [[ $PWD =~ ^/var/lib/jenkins/jobs/* ]]; then
  echo 'Please, run script on Jenkins.'
  exit 1
fi

set -e

# Cleaning
#rm -fr cmake_temp

git clean -f
git submodule update --init

#These following packages MUST be installed. See README of redemption project
#aptitude install build-essential bjam boost-build libboost-program-options-dev libboost-test-dev libssl-dev locales cmake

# BJAM Build Test
echo -e "using gcc : 8.0 : g++-8 -DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING ;\nusing clang : 6.0 : clang++-6.0 -DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING ;" > project-config.jam
valgrind_compiler=gcc-8
toolset_gcc=toolset=gcc-8
toolset_clang=toolset=clang-6.0
export FFMPEG_INC_PATH=/usr/local/include/ffmpeg/
export FFMPEG_LIB_PATH=/usr/local/lib/ffmpeg
export FFMPEG_LINK_MODE=static

export LSAN_OPTIONS=exitcode=0 # re-trace by valgrind

export BOOST_TEST_COLOR_OUTPUT=0

rm -rf bin

# export REDEMPTION_LOG_PRINT=1
export REDEMPTION_LOG_PRINT=0
export cxx_color=never

build()
{
    local e=0
    bjam -q "$@" || {
        e=$?
        export REDEMPTION_LOG_PRINT=1
        bjam -q "$@"
        exit 1
    }
}

# release for -Warray-bounds and not assert
#bjam -q $toolset_gcc cxxflags=-g
# multi-thread
big_mem='exe libs
  tests/capture
  tests/lib
  tests/server
  tests/client_mods
  tests/client_redemption/client_channels
  tests/mod/rdp.norec
  tests/mod/vnc.norec'
build -q $toolset_gcc cxxflags=-g -j2 ocr_tools
build -q $toolset_gcc cxxflags=-g $big_mem
build -q $toolset_gcc cxxflags=-g -j2

# debug with coverage
build -q $toolset_gcc debug -scoverage=on covbin=gcov-8

#bjam -a -q toolset=clang-6.0 -sNO_FFMPEG=1 san
# multi-thread
build -q $toolset_clang -sNO_FFMPEG=1 san -j3 ocr_tools
build -q $toolset_clang -sNO_FFMPEG=1 san $big_mem
build -q $toolset_clang -sNO_FFMPEG=1 san -j2

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


#set -o pipefail

# clang analyzer
CLANG_TIDY=clang-tidy-6.0 ./tools/c++-analyzer/clang-tidy \
  | sed -E '/^(.+\/|)modules\//,/\^/d'


# valgrind
#find ./bin/$gcc/release/tests/ -type d -exec \
#  ./tools/c++-analyzer/valgrind -qd '{}' \;
find ./bin/$valgrind_compiler/release/tests/ -type d -exec \
  parallel -j2 ./tools/c++-analyzer/valgrind -qd ::: '{}' +


# jsclient (emscripten)
cd projects/browser_client_JS
source ~/emsdk-master/emsdk_set_env.sh
rm -rf bin
version=$(clang++ --version | sed -E 's/^.*clang version ([0-9]+\.[0-9]+).*/\1/;q')
echo "using clang : $version : clang++ -DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING ;" > project-config.jam
if [ ! -d system_include/boost ]; then
    mkdir -p system_include
    ln -s /usr/include/boost/ system_include
fi
set -o pipefail
bjam -qj2 toolset=clang-$version debug |& sed '#^/var/lib/jenkins/jobs/redemption-future/workspace/##'
