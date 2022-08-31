#!/bin/bash

# Shell folder
shell_folder=$(cd "$(dirname "$0")" || exit;pwd)

openocd_bin_dir=""
openocd_root_dir=""
source_dir=""

# This script may in openocd/script dir or the same as openocd dir
cd ${shell_folder}
if [ -d "openocd" ]; then
    openocd_bin_dir=${shell_folder}/openocd/src
    openocd_root_dir=${shell_folder}/openocd
    source=${openocd_root_dir}/tcl/target/verisilicon
    echo "find openocd ${openocd_root_dir}"
elif [ -d "../../openocd" ]; then
    openocd_bin_dir=${shell_folder}/../../openocd/src
    openocd_root_dir=${shell_folder}/../../openocd
    source=${openocd_root_dir}/tcl/target/verisilicon
    echo "find openocd ${openocd_root_dir}"
elif [ -d "../../openocd_install" ]; then
    openocd_bin_dir=${shell_folder}/../../openocd_install/bin
    openocd_root_dir=${shell_folder}/../../openocd_install
    source=${openocd_root_dir}/share/openocd/scripts/target/verisilicon
    echo "find openocd ${openocd_root_dir}"
elif [ -d "openocd_install" ]; then
    openocd_bin_dir=${shell_folder}/openocd_install/bin
    openocd_root_dir=${shell_folder}/openocd_install
    source=${openocd_root_dir}/share/openocd/scripts/target/verisilicon
    echo "find openocd ${openocd_root_dir}"
else
    echo "can't find openocd, exit!!!!!"
    exit
fi

# Get config file by user input
config=""
if [[ $1  = "m33" ]]; then
    config=alius_m33.cfg
    echo "Debug m33"
elif [[ $1  = "lp" ]]; then
    config=alius_lp.cfg
    echo "Debug lp"
elif [[ $1  = "hp" ]]; then
    config=alius_hp.cfg
    echo "Debug hp"
else
    echo "please specify which module to debug, like ./run_openocd.sh m33"
    exit
fi

# Change to project root dir for monitor pwd is root dir
cd ${openocd_root_dir}/..

#sudo gdb --args ${openocd} -f ${m33_config} -f ${target_config} -s ${source} --debug
sudo ${openocd_bin_dir}/openocd -f ${config} -s ${source}
 
 