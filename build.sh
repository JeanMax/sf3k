#!/bin/bash

set -ex

### Requirements
# 1. CMake 3.20 or later
# 2. Raspberry Pi Pico C SDK

# > [!IMPORTANT]
# > The project assumes the Pico SDK is installed in your home directory and in a
# > folder named `.pico-sdk`. I.e., `/Users/dev/.pico-sdk/`
if ! test -e ~/.pico-sdk; then
    ln -sv /usr/share/pico-sdk ~/.pico-sdk
fi

# Default to debug build
if test "$RELEASE"; then
    REL=Release
else
    REL=Debug
fi

BUILD_ROOT=./build
BUILD_DIR=$BUILD_ROOT/$REL
mkdir -p $BUILD_ROOT

# export MAKEFLAGS=-j$(nproc)
export CMAKE_EXPORT_COMPILE_COMMANDS=ON

# Clean targets
test "$MRPROPER" && rm -rf $BUILD_ROOT && mkdir -p $BUILD_ROOT
test "$CLEAN" && cmake --build $BUILD_DIR --target clean

# Within the project root, use CMake to generate the build scripts
test -d $BUILD_DIR || cmake -DCMAKE_BUILD_TYPE=$REL -B $BUILD_DIR

# Build the project
cmake --build $BUILD_DIR -j
# -DCMAKE_BUILD_TYPE=$REL

cp $BUILD_DIR/compile_commands.json .
