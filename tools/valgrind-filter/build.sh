#!/bin/sh
g++ -Wall -Wextra -std=c++11 -O3 -march=native valgrind-filter.cpp -o valgrind-filter
