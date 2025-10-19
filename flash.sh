#!/bin/bash

set -e

BUILD_DIR=./build
TO_FLASH=$(find $BUILD_DIR -maxdepth 1 -name '*.uf2')


if ! test -e "$TO_FLASH"; then
    ./build.sh
    TO_FLASH=$(find $BUILD_DIR -maxdepth 1 -name '*.uf2')
fi

# Check if pico is here
PICO_INFO=$(sudo picotool info -a || true)

if grep -q 'appears to have a USB serial connection' <<< "$PICO_INFO"; then
    # need to reboot to BOOTSEL
    sudo picotool reboot -f -u
    sleep 1
    PICO_INFO=$(sudo picotool info -a || true)
fi

if grep -qE 'boot type:.*bootsel' <<< "$PICO_INFO"; then
    echo "Found pi in BOOTSEL mode"
else
    echo "Couldn't bring pi to a flashable state, abort."
    exit 42
fi


# Flash the uf2 file
sudo picotool load --update "$TO_FLASH"

# Manual way:
# sudo mount /dev/sdb1 /mnt
# sudo cp blink/blink.uf2 /mnt
# sync
# sudo umount /mnt


sudo picotool info

# Back to app?
sudo picotool reboot -a

echo YAY
