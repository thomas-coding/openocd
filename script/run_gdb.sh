#!/bin/bash

# Define the toolchain path
toolchian_path=~/.toolchain
if [[ $1  = "m33" ]]; then
export PATH="${toolchian_path}/gcc-arm-none-eabi-10.3-2021.07/bin/:$PATH"
elif [[ $1  = "lp" ]]; then
export PATH="${toolchian_path}/gcc-arm-10.3-2021.07-x86_64-arm-none-eabi/bin/:$PATH"
elif [[ $1  = "hp" ]]; then
export PATH="${toolchian_path}/gcc-arm-10.3-2021.07-x86_64-arm-none-eabi/bin/:$PATH"
else
    echo "please specify which module to debug, like ./run_gdb.sh m33"
    exit
fi

# Shell folder
shell_folder=$(cd "$(dirname "$0")" || exit;pwd)

# This script may in openocd/script dir or the same as openocd dir
project_dir=""
cd ${shell_folder}
if [ -d "openocd" ]; then
    project_dir=${shell_folder}
    echo "find project ${openocd_dir}"
elif [ -d "../../openocd" ]; then
    project_dir=${shell_folder}/../..
    echo "find project ${openocd_dir}"
elif [ -d "../../openocd_install" ]; then
    project_dir=${shell_folder}/../..
    echo "find project ${openocd_dir}"
elif [ -d "openocd_install" ]; then
    project_dir=${shell_folder}
    echo "find project ${openocd_dir}"
else
    echo "can't find project dir, exit!!!!!"
    exit
fi

cd ${project_dir}

# m33 elf
tfm_elf=${project_dir}/out/alius_lp/intermediate/pigweed/alius_m33_size_optimized/obj/trusted-firmware-m/bin/tfm_s.elf
freerots_elf=${project_dir}/out/alius_lp/intermediate/pigweed/alius_m33_size_optimized/obj/freertos/FreeRTOS/project/alius_m33/bin/alius_m33_elf.elf

# lp elf
bl2_elf=${project_dir}/out/alius_lp/intermediate/atf/alius/release/bl2/bl2.elf
lp_tee_elf=${project_dir}/out/alius_lp/intermediate/optee/optee_os/core/tee.elf
lp_uboot_elf=${project_dir}/out/alius_lp/intermediate/uboot/u-boot
lp_kernel_elf=${project_dir}/out/alius_lp/intermediate/kernel/vmlinux

# hp elf
hp_tee_elf=${project_dir}/out/alius_hp/intermediate/optee/optee_os/core/tee.elf
hp_uboot_elf=${project_dir}/out/alius_hp/intermediate/uboot/u-boot
hp_kernel_elf=${project_dir}/out/alius_hp/intermediate/kernel/vmlinux

# defalut is localhost, change ip for connect remote gdb server
# ip=10.10.13.190
server_ip=127.0.0.1
#-ex "monitor reset halt"
# Get config file by user input
if [[ $1  = "m33" ]]; then
    # run Gdb
    arm-none-eabi-gdb \
    -ex "file ${tfm_elf}" \
    -ex "target extended-remote ${server_ip}:3333" \
    -ex "add-symbol-file ${freerots_elf}" \
    -ex "add-symbol-file ${tfm_elf}" \
    -q
elif [[ $1  = "lp" ]]; then
    # run Gdb
    arm-none-eabi-gdb \
    -ex "file ${lp_uboot_elf}" \
    -ex "target extended-remote ${server_ip}:3333" \
    -ex "add-symbol-file ${bl2_elf}" \
    -ex "add-symbol-file ${lp_tee_elf}" \
    -ex "add-symbol-file ${lp_uboot_elf}" \
    -ex "add-symbol-file ${lp_kernel_elf}" \
    -q
elif [[ $1  = "hp" ]]; then
    # run Gdb
    arm-none-eabi-gdb \
    -ex "file ${hp_uboot_elf}" \
    -ex "target extended-remote ${server_ip}:3333" \
    -ex "add-symbol-file ${hp_tee_elf}" \
    -ex "add-symbol-file ${hp_uboot_elf}" \
    -ex "add-symbol-file ${hp_kernel_elf}" \
    -q
else
    echo "please specify which module to debug, like ./run_gdb.sh m33"
    exit
fi
