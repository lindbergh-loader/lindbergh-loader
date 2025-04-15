#!/usr/bin/env bash

APPIMAGEDIRNAME=lindbergh-loader.AppDir
OUTPUT_FOLDER=$(realpath "./$APPIMAGEDIRNAME")

mkdir build-deps
cd build-deps

# Build openal
wget https://github.com/kcat/openal-soft/archive/refs/tags/openal-soft-1.17.2.tar.gz
tar -xzf openal-soft-1.17.2.tar.gz
cd openal-soft-openal-soft-1.17.2/build

CFLAGS="-m32 -mstackrealign" CXXFLAGS="-m32 -std=c++14 -mstackrealign" cmake ../ -DALSOFT_BACKEND_PULSEAUDIO=ON -DALSOFT_BACKEND_ALSA=ON -DALSOFT_BACKEND_JACK=OFF -DALSOFT_BACKEND_SNDIO=ON

make -j4
sudo cp libopenal.so.1.17.2 /usr/lib/i386-linux-gnu/libopenal.so.0
cd ../../

# Build libSSL and libCrypto
wget https://github.com/openssl/openssl/archive/refs/tags/OpenSSL_0_9_7.tar.gz
tar -xzf OpenSSL_0_9_7.tar.gz
cd openssl-OpenSSL_0_9_7
cp ../../scripts/appimage/patches/openssl-0.9.7.patch .
./Configure shared 386 no-asm no-krb5 linux-pentium -m32
patch -p1 < openssl-0.9.7.patch
CFLAGS=-m32 make 
sudo cp libcrypto.so.0.9.7 /usr/lib/i386-linux-gnu
sudo cp libssl.so.0.9.7 /usr/lib/i386-linux-gnu

