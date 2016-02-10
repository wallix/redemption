#!/bin/sh

cd "`dirname "$0"`"

g++ tmalloc.cpp -O3 -std=c++11 -DNDEBUG -march=native -shared -fPIC -ldl -o libtmalloc.so
