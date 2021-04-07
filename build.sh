#!/bin/bash

mkdir -p ./build
mkdir -p ./obj
clang++ -Wall -std=c++17 -g3 src/main.cpp src/error_handling.cpp src/operators.cpp src/token.cpp src/lexing.cpp src/parsing.cpp -o ./build/witcc
