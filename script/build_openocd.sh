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

openocd_install=${openocd_dir}/openocd_install

./bootstrap
./configure --enable-jlink CFLAGS='-g -O0' \
--prefix=${openocd_install} \
--exec-prefix=${openocd_install}

# Delete install
rm -rf ${openocd_install}
rm -rf ${openocd_dir}/openocd_install.tar.gz

# Build
make -j4

# Install
make install

# Copy to install
cd ${openocd_install}
mkdir -p ${openocd_install}/script
cp -f ${openocd_dir}/script/run_openocd.sh ${openocd_install}/script
cp -f ${openocd_dir}/script/run_gdb.sh ${openocd_install}/script

cd ${openocd_dir}
tar -zcf ${openocd_dir}/openocd_install.tar.gz openocd_install
