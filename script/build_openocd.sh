#!/bin/bash

# Shell folder
shell_folder=$(cd "$(dirname "$0")" || exit;pwd)

# This script may in openocd/script dir or the same as openocd dir
cd ${shell_folder}
openocd_dir=""
if [ -d "openocd" ]; then
    openocd_dir=${shell_folder}/openocd
    echo "find openocd ${openocd_dir}"
elif [ -d "../../openocd" ]; then
    openocd_dir=${shell_folder}/../../openocd
    echo "find openocd ${openocd_dir}"
else
    echo "can't find openocd, exit!!!!!"
    exit
fi

# First time config it. only execute once
cd ${openocd_dir}
./bootstrap
./configure --enable-jlink CFLAGS='-g -O0'

# Build
make -j4
