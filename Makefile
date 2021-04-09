CXX = clang++
CPP_VERSION = c++17
CFLAGS = -std=$(CPP_VERSION) -Wall -Wextra


witcc : obj/main.o obj/error_handling.o obj/operators.o obj/token.o obj/lexing.o obj/parsing.o
	$(CXX) $(CFLAGS) -o $@ $?


obj :
	mkdir -p obj


obj/main.o : src/main.cpp obj
	$(CXX) $(CFLAGS) -c $< -o $@

obj/error_handling.o : src/error_handling.cpp src/error_handling.hpp src/lexing.hpp src/parsing.hpp src/node.hpp src/error.hpp obj
	$(CXX) $(CFLAGS) -c $< -o $@

obj/operators.o : src/parsing/operators.cpp src/operators.hpp obj
	$(CXX) $(CFLAGS) -c $< -o $@

obj/token.o : src/parsing/token.cpp src/lexing.hpp obj
	$(CXX) $(CFLAGS) -c $< -o $@

obj/lexing.o : src/parsing/lexing.cpp src/lexing.hpp src/error_handling.hpp src/error.hpp obj
	$(CXX) $(CFLAGS) -c $< -o $@

obj/parsing.o : src/parsing/parsing.cpp src/parsing.hpp src/node.hpp src/operators.hpp src/error_handling.hpp src/error.hpp obj
	$(CXX) $(CFLAGS) -c $< -o $@


clean :
	rm ./obj/*
	rm ./witcc
