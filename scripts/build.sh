#!/bin/bash

set -e

os=$(uname -o | sed "s#GNU/##g" | tr '[:upper:]' '[:lower:]')
if [ -n "$TRAVIS" ] || [ -n "$CI" ]; then
  if [ "$os" != "freebsd" ]; then
    export DEBIAN_FRONTEND=noninteractive
    sudo apt update || true
    sudo apt install -y gcc-arm-linux-gnueabihf || true
    sudo apt install -y cppcheck || true
    sudo apt install -y clang || true
    sudo apt install -y gcc || true
    sudo apt install -y clang-tidy || true
    sudo apt install -y clang-format || true
    sudo apt install -y clang-tools || true
    sudo apt install -y clang-format-11 || true
    sudo apt install -y cmake || true
    sudo apt install -y libsdl2-dev || true
    sudo apt install -y libsdl2-mixer-dev || true
    sudo apt install -y libsdl2-image-dev || true
    sudo ln -sf make /usr/bin/gmake
    ub_ld_path="lib/x86_64-linux-gnu"
    sudo ln -sf ../../lib/x86_64-linux-gnu/libSDL2.so /usr/lib64/libSDL2.so
    sudo ln -sf ../../$ub_ld_path/libSDL2_mixer.so /usr/lib64/libSDL2_mixer.so
    sudo ln -sf ../../$ub_ld_path/libSDL2_image.so /usr/lib64/libSDL2_image.so
  fi
fi

arg1="$1"
if [ "$arg1" == "clean" ]; then
  rm -rf build
fi

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Release ..
make -j8 VERBOSE=1
cd -

if [ "$arg1" == "clean" ]; then
  rm -rf build
fi

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Release ..
make -j8 VERBOSE=1
cd -
