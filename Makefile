all: build/main

test: build/test
	for (( i=0; i<1000; i++ )) do ./build/test; done

build/main: main.cpp lz77.h lz77.cpp
	mkdir -p build && g++ -O2 main.cpp lz77.cpp -o build/main

build/test: test.cpp lz77.h lz77.cpp
	mkdir -p build && g++ -O2 test.cpp lz77.cpp -o build/test

