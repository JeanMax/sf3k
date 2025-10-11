#!/bin/bash

set -e

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
test -d $BUILD_DIR || cmake -B $BUILD_DIR


#TODO: pass these
# cmake -DPICO_BOARD=pico2_w -DPICO_PLATFORM=rp2350 \
#       -DWIFI_SSID=Linternet -DWIFI_PASSWORD=internet974 \

# Build the project
cmake --build $BUILD_DIR -j

if ! grep 'I/usr/arm-none-eabi/include' \
     $BUILD_DIR/compile_commands.json; then
    sed -i -E 's|(-I/usr/share/pico-sdk/.*main\.c)|-I/usr/arm-none-eabi/include \1|' \
        $BUILD_DIR/compile_commands.json
fi
