#!/bin/bash

set -ex



WIFI_SSID=${WIFI_SSID:-Linternet}
if ! test "$WIFI_PASSWORD"; then
    echo "You need to pass \$WIFI_PASSWORD (and eventually change \$WIFI_SSID=$WIFI_SSID)"
    exit 42
fi
if ! test "$BREW_KEY"; then
    echo "You need to pass \$BREW_KEY (and eventually change \$WIFI_SSID=$WIFI_SSID)"
    exit 42
fi



### Requirements
# 1. CMake 3.20 or later
# 2. Raspberry Pi Pico C SDK

# > [!IMPORTANT]
# > The project assumes the Pico SDK is installed in your home directory and in a
# > folder named `.pico-sdk`. I.e., `/Users/dev/.pico-sdk/`
if ! test -e ~/.pico-sdk; then
    ln -sv /usr/share/pico-sdk ~/.pico-sdk
fi
export PICO_SDK_PATH=~/.pico-sdk

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
test -d $BUILD_DIR || cmake -DCMAKE_BUILD_TYPE=$REL \
                            -DWIFI_SSID="$WIFI_SSID" \
                            -DWIFI_PASSWORD="$WIFI_PASSWORD" \
                            -DBREW_KEY="$BREW_KEY" \
                            -B $BUILD_DIR

# Build the project
cmake --build $BUILD_DIR -j
# -DCMAKE_BUILD_TYPE=$REL

cp $BUILD_DIR/compile_commands.json .
