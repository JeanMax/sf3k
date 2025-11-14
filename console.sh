#!/bin/bash -x

sudo minicom -C /tmp/sf3k.log \
     -D /dev/ttyACM? \
     -b 115200
