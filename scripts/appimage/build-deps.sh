#!/usr/bin/env bash

APPIMAGEDIRNAME=lindbergh-loader.AppDir
OUTPUT_FOLDER=$(realpath "./$APPIMAGEDIRNAME")

mkdir build-deps
cd build-deps

# Build SDL3
git clone https://github.com/libsdl-org/SDL.git
cd SDL
mkdir build
cd build
cmake ../ -DCMAKE_C_FLAGS=-m32 -DCMAKE_INSTALL_PREFIX=/usr
make -j4
sudo make install
cd ../../

# Build SDL3_ttf
git clone https://github.com/libsdl-org/SDL_ttf.git
cd SDL_ttf
mkdir build
cd build
cmake ../ -DCMAKE_C_FLAGS=-m32 -DCMAKE_INSTALL_PREFIX=/usr
make -j4
sudo make install
cd ../../

# Build SDL3_image
git clone https://github.com/libsdl-org/SDL_image.git
cd SDL_image
mkdir build
cd build
cmake ../ -DCMAKE_C_FLAGS=-m32 -DCMAKE_INSTALL_PREFIX=/usr
make -j4
sudo make install
cd ../../


# Build libFAudio
git clone https://github.com/FNA-XNA/FAudio.git
cd FAudio
mkdir build
cd build
cmake ../ -DCMAKE_C_FLAGS=-m32 -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=off
make -j4
sudo make install
cd ../../../


