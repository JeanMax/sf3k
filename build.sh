#!/bin/bash

set -ex

BUILD_DIR=./build

### Requirements
# 1. CMake 3.20 or later
# 2. Raspberry Pi Pico C SDK

# > [!IMPORTANT]
# > The project assumes the Pico SDK is installed in your home directory and in a
# > folder named `.pico-sdk`. I.e., `/Users/dev/.pico-sdk/`
if ! test -e ~/.pico-sdk; then
    ln -sv /usr/share/pico-sdk ~/.pico-sdk
fi

test "$CLEAN" && rm -rf $BUILD_DIR

# export MAKEFLAGS=-j$(nproc)
export CMAKE_EXPORT_COMPILE_COMMANDS=ON

# Within the project root, use CMake to generate the build scripts
test -d $BUILD_DIR || cmake $(test "$RELEASE" || echo -DCMAKE_BUILD_TYPE=Debug) -B $BUILD_DIR

# Build the project
cmake --build $BUILD_DIR -j
