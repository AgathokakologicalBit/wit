#!/bin/bash


clang++ -Wall -std=c++17 main.cpp token.cpp lexing.cpp parsing.cpp error_handling.cpp -o ./witcc
