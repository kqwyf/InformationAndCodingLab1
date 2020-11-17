SHELL := /bin/bash

all: build/main

test: build/test
	./build/test

realtest: build/main
	echo "================="; \
	echo "Test on train.txt"; \
	filesize=$$(wc -c data/train.txt | cut -d ' ' -f 1); \
	time ./build/main -7 -C --sb 32767 --lb 32767 -i data/train.txt -o data/train.77 && time ./build/main -7 -D --sb 32767 --lb 32767 -i data/train.77 -o data/train.out; \
	if ! diff data/train.txt data/train.out; then echo "LZ77 failed."; exit 1; else newsize=`wc -c data/train.77 | cut -d ' ' -f 1`; echo LZ77 compression rate: `awk "BEGIN {printf \\"%.2f\\n\\", $${newsize} / $${filesize}}"`; fi; \
	time ./build/main -p -C -n 8 --sb 32767 --lb 32767 -i data/train.txt -o data/train.77p && time ./build/main -p -D --sb 32767 --lb 32767 -i data/train.77p -o data/train.out; \
	if ! diff data/train.txt data/train.out; then echo "LZ77 parallel failed."; exit 1; else newsize=`wc -c data/train.77p | cut -d ' ' -f 1`; echo LZ77 parallel compression rate: `awk "BEGIN {printf \\"%.2f\\n\\", $${newsize} / $${filesize}}"`; fi; \
	time ./build/main -8 -C --ds 32767 -i data/train.txt -o data/train.78 && time ./build/main -8 -D --ds 32767 -i data/train.78 -o data/train.out; \
	if ! diff data/train.txt data/train.out; then echo "LZ78 failed."; exit 1; else newsize=`wc -c data/train.78 | cut -d ' ' -f 1`; echo LZ78 compression rate: `awk "BEGIN {printf \\"%.2f\\n\\", $${newsize} / $${filesize}}"`; fi; \
	time ./build/main -w -C --ds 32767 -i data/train.txt -o data/train.w && time ./build/main -w -D --ds 32767 -i data/train.w -o data/train.out; \
	if ! diff data/train.txt data/train.out; then echo "LZW failed."; exit 1; else newsize=`wc -c data/train.w | cut -d ' ' -f 1`; echo LZW compression rate: `awk "BEGIN {printf \\"%.2f\\n\\", $${newsize} / $${filesize}}"`; fi; \
	rm data/train.out data/train.77 data/train.78 data/train.77p data/train.w
	echo "================="; \
	echo "Test on intro.txt"; \
	filesize=$$(wc -c data/intro.txt | cut -d ' ' -f 1); \
	time ./build/main -7 -C --sb 256 --lb 256 -i data/intro.txt -o data/intro.77 && time ./build/main -7 -D --sb 256 --lb 256 -i data/intro.77 -o data/intro.out; \
	if ! diff data/intro.txt data/intro.out; then echo "LZ77 failed."; exit 1; else newsize=`wc -c data/intro.77 | cut -d ' ' -f 1`; echo LZ77 compression rate: `awk "BEGIN {printf \\"%.2f\\n\\", $${newsize} / $${filesize}}"`; fi; \
	time ./build/main -p -C -n 8 --sb 256 --lb 256 -i data/intro.txt -o data/intro.77p && time ./build/main -p -D --sb 256 --lb 256 -i data/intro.77p -o data/intro.out; \
	if ! diff data/intro.txt data/intro.out; then echo "LZ77 parallel failed."; exit 1; else newsize=`wc -c data/intro.77p | cut -d ' ' -f 1`; echo LZ77 parallel compression rate: `awk "BEGIN {printf \\"%.2f\\n\\", $${newsize} / $${filesize}}"`; fi; \
	time ./build/main -8 -C --ds 32767 -i data/intro.txt -o data/intro.78 && time ./build/main -8 -D --ds 32767 -i data/intro.78 -o data/intro.out; \
	if ! diff data/intro.txt data/intro.out; then echo "LZ78 failed."; exit 1; else newsize=`wc -c data/intro.78 | cut -d ' ' -f 1`; echo LZ78 compression rate: `awk "BEGIN {printf \\"%.2f\\n\\", $${newsize} / $${filesize}}"`; fi; \
	time ./build/main -w -C --ds 32767 -i data/intro.txt -o data/intro.w && time ./build/main -w -D --ds 32767 -i data/intro.w -o data/intro.out; \
	if ! diff data/intro.txt data/intro.out; then echo "LZW failed."; exit 1; else newsize=`wc -c data/intro.w | cut -d ' ' -f 1`; echo LZW compression rate: `awk "BEGIN {printf \\"%.2f\\n\\", $${newsize} / $${filesize}}"`; fi; \
	rm data/intro.out data/intro.77 data/intro.78 data/intro.77p data/intro.w

build/main: main.cpp lz77.cpp lz78.cpp lzw.cpp lz77.h lz78.h lzw.h
	mkdir -p build && g++ -O2 main.cpp lz77.cpp lz78.cpp lzw.cpp -o build/main -g -lpthread

build/test: test.cpp lz77.cpp lz78.cpp lzw.cpp lz77.h lz78.h lzw.h
	mkdir -p build && g++ -O2 test.cpp lz77.cpp lz78.cpp lzw.cpp -o build/test -g -lpthread
