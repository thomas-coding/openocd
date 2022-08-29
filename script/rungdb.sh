#!/bin/bash

# NOTICE: copy this script to project dir

# Define the toolchain path
export PATH="/home/cn1396/.toolchain/gcc-arm-none-eabi-10.3-2021.07/bin/:$PATH"

# Shell folder
shell_folder=$(cd "$(dirname "$0")" || exit;pwd)

# run Gdb
arm-none-eabi-gdb \
-ex 'target remote localhost:3333' \
-q
