all: build/main

test: build/test
	./build/test

build/main: main.cpp lz77.cpp lz78.cpp lzw.cpp lz77.h lz78.h lzw.h
	mkdir -p build && g++ -O2 main.cpp lz77.cpp lz78.cpp lzw.cpp -o build/main -g -lpthread

build/test: test.cpp lz77.cpp lz78.cpp lzw.cpp lz77.h lz78.h lzw.h
	mkdir -p build && g++ -O2 test.cpp lz77.cpp lz78.cpp lzw.cpp -o build/test -g -lpthread
