#!/bin/bash

# Shell folder
shell_folder=$(cd "$(dirname "$0")" || exit;pwd)
openocd_dir=""

# This script may in openocd/script dir or the same as openocd dir
cd ${shell_folder}
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

# Get config file by user input
config=""
if [[ $1  = "m33" ]]; then
    config=${openocd_dir}/tcl/target/verisilicon/alius_m33.cfg
    echo "Debug m33"
elif [[ $1  = "lp" ]]; then
    config=${openocd_dir}/tcl/target/verisilicon/alius_lp.cfg
    echo "Debug lp"
else
    echo "please specify which module to debug, like ./run_openocd.sh m33"
    exit
fi

# Change to project root dir for monitor pwd is root dir
cd ${openocd_dir}/..

#sudo gdb --args ${openocd} -f ${m33_config} -f ${target_config} -s ${source} --debug
sudo ${openocd_dir}/src/openocd -f ${config}
 