#!/bin/bash

# sanitizers="-fsanitize=address -fsanitize=leak -fsanitize=undefined"
# sanitizers="-fsanitize=safe-stack"
# sanitizers="-fsanitize=undefined -fsanitize=shadow-call-stack -fsanitize=integer -flto"
# sanitizers="-fsanitize=undefined -fsanitize=memory -fsanitize=shadow-call-stack -fsanitize=integer -flto"

clang++ src/app/app.cpp -shared -o build/app.so -g -I/usr/local/include build/clay.o $(pkg-config --cflags --libs sdl3) --std=gnu++20 -Wno-vla-cxx-extension -Wno-c++11-narrowing -lSDL3_ttf -lSDL3_image -fPIC $sanitizers