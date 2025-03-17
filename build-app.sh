#!/bin/bash

clang++ src/app/app.cpp -shared -o build/app.so -g -I/usr/local/include build/clay.o $(pkg-config --cflags --libs sdl3) --std=gnu++20 -Wno-vla-cxx-extension -Wno-c++11-narrowing -lSDL3_ttf -lSDL3_image -fPIC