#!/bin/bash

# shell folder
shell_folder=$(cd "$(dirname "$0")" || exit;pwd)

# First time config it. only execute once
cd ${shell_folder}/openocd
./bootstrap
./configure --enable-jlink CFLAGS='-g -O0'

# Build
make -j4
