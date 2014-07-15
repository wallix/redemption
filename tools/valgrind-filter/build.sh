#!/bin/sh

make CXXFLAGS+='-Os -march=native -std=c++11' valgrind-filter
