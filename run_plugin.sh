#!/bin/sh

clang -Xclang -load -Xclang .libs/libprintfunctionnames.so -Xclang -plugin -Xclang print-fns -Ibson examples/bcon-speed.c
