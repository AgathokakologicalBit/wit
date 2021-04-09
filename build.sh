#!/bin/bash

mkdir -p ./build
mkdir -p ./obj
clang++ -Wall -Wextra -std=c++17 -g3 src/main.cpp src/error_handling.cpp src/parsing/operators.cpp src/parsing/token.cpp src/parsing/lexing.cpp src/parsing/parsing.cpp -o ./build/witcc
