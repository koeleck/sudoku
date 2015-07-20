.PHONY: all clean

all: test

test: main.cpp grid.cpp grid.h
	g++ -std=c++14 -o $@ $^ -Wall -Wextra -pthread -O2

clean:
	rm -f test
