#!/bin/bash

# Define the toolchain path
export PATH="/home/cn1396/.toolchain/gcc-arm-none-eabi-10.3-2021.07/bin/:$PATH"

# Shell folder
shell_folder=$(cd "$(dirname "$0")" || exit;pwd)

# This script may in openocd/script dir or the same as openocd dir
project_dir=""
cd ${shell_folder}
if [ -d "openocd" ]; then
    project_dir=${shell_folder}
    echo "find project ${project_dir}"
elif [ -d "../../openocd" ]; then
    project_dir=${shell_folder}/../..
    echo "find project ${project_dir}"
elif [ -d "../../openocd_install" ]; then
    project_dir=${shell_folder}/../..
    echo "find project ${project_dir}"
elif [ -d "openocd_install" ]; then
    project_dir=${shell_folder}
    echo "find project ${project_dir}"
else
    echo "can't find project dir, exit!!!!!"
    exit
fi

cd ${project_dir}

echo "starting dump assemble, wait ..."
rm -f bl2.asm tee.asm freertos.asm tfm.asm
arm-none-eabi-objdump -xD ${project_dir}/out/alius_lp/intermediate/atf/alius/release/bl2/bl2.elf > bl2.asm
arm-none-eabi-objdump -xD ${project_dir}/out/alius_lp/intermediate/optee/optee_os/core/tee.elf > tee.asm
arm-none-eabi-objdump -xD ${project_dir}/out/alius_lp/intermediate/pigweed/alius_m33_size_optimized/obj/freertos/FreeRTOS/project/alius_m33/bin/alius_m33_elf.elf > freertos.asm
arm-none-eabi-objdump -xD ${project_dir}/out/alius_lp/intermediate/pigweed/alius_m33_size_optimized/obj/trusted-firmware-m/bin/tfm_s.elf > tfm.asm
echo "dump done"
