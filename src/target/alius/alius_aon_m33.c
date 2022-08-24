/* SPDX-License-Identifier: GPL-2.0-or-later */

/***************************************************************************
 *   Copyright (C) 2009 Jinping Wu <wunekky@gmail.com>             *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <helper/log.h>
#include "alius_aon_m33.h"
#include "alius_rom_table.h"

#include "jtag/interface.h"
#include "target/arm.h"
#include "target/arm_adi_v5.h"
#include "target/arm_coresight.h"
#include "jtag/swd.h"
#include "transport/transport.h"
#include <helper/align.h>
#include <helper/jep106.h>
#include <helper/time_support.h>
#include <helper/list.h>
#include <helper/jim-nvp.h>

extern struct adiv5_dap *global_dap;
uint32_t ap_init = 0;

/* from address & 0xFFFFFFF0, read 4 * 4 bytes, 16 bytes once
 * only need set tar once, and read from bd0 - bd3
 */
int aon_m33_read_memory_16bytes_by_bd(uint32_t address, uint32_t *value) {
	int retval = ERROR_OK;
	uint32_t temp;

	struct adiv5_ap *ap = dap_get_ap(global_dap, TOP_AON_APBAP);

	if((address & 0xf) != 0) {
		return ERROR_FAIL;		
	}

	//write aon tar. value is m33 tar
	retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + AP_TAR);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	//write aon drw. value is address. so update m33 tar to address
	retval = dap_queue_ap_write(ap, AP_DRW, address);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	for(int i = 0; i < 4; i++) {
		//write aon tar. value is m33 BD
		retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + AP_BD0 + 4 * i);
		if (retval == ERROR_OK)
			retval = dap_run(ap->dap);
		
		//get the value
		retval = dap_queue_ap_read(ap, AP_DRW, &temp);
		if (retval == ERROR_OK)
			retval = dap_run(ap->dap);
		
		value[i] = temp;
	}

	return retval;
}

/* from address & 0xFFFFFFF0, write 4 * 4 bytes, 16bytes once
 * only need set tar once, and read from bd0 - bd3
 */
int aon_m33_write_memory_16bytes_by_bd(uint32_t address, uint32_t *value) {
	int retval = ERROR_OK;

	struct adiv5_ap *ap = dap_get_ap(global_dap, TOP_AON_APBAP);

	if((address & 0xf) != 0) {
		return ERROR_FAIL;		
	}

	//write aon tar. value is m33 tar
	retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + AP_TAR);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	//write aon drw. value is address. so update m33 tar to address
	retval = dap_queue_ap_write(ap, AP_DRW, address);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	for(int i = 0; i < 4; i++) {
		//write aon tar. value is m33 DAR
		retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + AP_BD0 + 4 * i);
		if (retval == ERROR_OK)
			retval = dap_run(ap->dap);
		
		//write value
		retval = dap_queue_ap_write(ap, AP_DRW, value[i]);
		if (retval == ERROR_OK)
			retval = dap_run(ap->dap);
	}

	return retval;
}

/* from address & 0xFFFFFC00, read 256 * 4 bytes, 1k once
 * only need set tar once, and read from dar0 - dar255
 */
int aon_m33_read_memory_1k_by_dar(uint32_t address, uint32_t *value) {
	int retval = ERROR_OK;
	uint32_t temp;

	struct adiv5_ap *ap = dap_get_ap(global_dap, TOP_AON_APBAP);

	if((address & 0x3ff) != 0) {
		return ERROR_FAIL;		
	}

	//write aon tar. value is m33 tar
	retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + AP_TAR);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	//write aon drw. value is address. so update m33 tar to address
	retval = dap_queue_ap_write(ap, AP_DRW, address);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	for(int i = 0; i < 256; i++) {
		//write aon tar. value is m33 DAR
		retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + AP_DAR0 + 4 * i);
		if (retval == ERROR_OK)
			retval = dap_run(ap->dap);
		
		//get the value from m33 drw
		retval = dap_queue_ap_read(ap, AP_DRW, &temp);
		if (retval == ERROR_OK)
			retval = dap_run(ap->dap);
		
		value[i] = temp;
	}

	return retval;
}

/* from address & 0xFFFFFC00, write 256 * 4 bytes, 1k once
 * only need set tar once, and read from dar0 - dar255
 */
int aon_m33_write_memory_1k_by_dar(uint32_t address, uint32_t *value) {
	int retval = ERROR_OK;

	struct adiv5_ap *ap = dap_get_ap(global_dap, TOP_AON_APBAP);

	if((address & 0x3ff) != 0) {
		return ERROR_FAIL;		
	}

	//write aon tar. value is m33 tar
	retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + AP_TAR);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	//write aon drw. value is address. so update m33 tar to address
	retval = dap_queue_ap_write(ap, AP_DRW, address);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	for(int i = 0; i < 256; i++) {
		//write aon tar. value is m33 DAR
		retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + AP_DAR0 + 4 * i);
		if (retval == ERROR_OK)
			retval = dap_run(ap->dap);
		
		//write value the value to m33 drw
		retval = dap_queue_ap_write(ap, AP_DRW, value[i]);
		if (retval == ERROR_OK)
			retval = dap_run(ap->dap);
	}

	return retval;
}

int aon_m33_read_memory_drw(uint32_t address, uint32_t *value) {
	int retval = ERROR_OK;

	struct adiv5_ap *ap = dap_get_ap(global_dap, TOP_AON_APBAP);

	//write aon tar. value is m33 tar
	retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + AP_TAR);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	//write aon drw. value is address. so update m33 tar to address
	retval = dap_queue_ap_write(ap, AP_DRW, address);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	//write aon tar. value is m33 drw
	retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + AP_DRW);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);
	
	//get the value from m33 drw
	retval = dap_queue_ap_read(ap, AP_DRW, value);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);
	
	return retval;
}

int aon_m33_write_memory_dwr(uint32_t address, uint32_t value) {
	int retval = ERROR_OK;

	struct adiv5_ap *ap = dap_get_ap(global_dap, TOP_AON_APBAP);

	//write aon tar. value is m33 tar
	retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + AP_TAR);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	//write aon drw. value is address. so update m33 tar to address
	retval = dap_queue_ap_write(ap, AP_DRW, address);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	//write aon tar. value is m33 drw
	retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + AP_DRW);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);
	
	//write value to drw
	retval = dap_queue_ap_write(ap, AP_DRW, value);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);
	
	return retval;
}

void alius_aon_m33_init_ap(struct adiv5_dap *dap) {
	dap->ap[0].ap_num = TOP_BASE;
	dap->ap[1].ap_num = AON_M33_AHBAP;
	dap->adi_version = 6;
}

COMMAND_HANDLER(handle_info_command)
{
	command_print(CMD, "m33 info is hello ....");
	return ERROR_OK;
}

COMMAND_HANDLER(handle_md_command)
{
	uint32_t value, address, retval;

	if (CMD_ARGC != 1)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], address);

	if(!ap_init) {
		alius_aon_m33_init_ap(global_dap);
		ap_init = 1;
	}

	retval = aon_m33_read_memory_drw(address, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read 0x%08x fail", address);
		return retval;
	}

	command_print(CMD, "0x%08x:  0x%08x", address, value);
	return ERROR_OK;
}

COMMAND_HANDLER(handle_mw_command)
{
	uint32_t value, address, retval;

	if (CMD_ARGC != 2)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], address);
	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[1], value);


	if(!ap_init) {
		alius_aon_m33_init_ap(global_dap);
		ap_init = 1;
	}

	retval = aon_m33_write_memory_dwr(address, value);
	if(retval != ERROR_OK) {
		command_print(CMD, "write 0x%08x fail", address);
		return retval;
	}

	command_print(CMD, "0x%08x:  0x%08x", address, value);
	return ERROR_OK;
}

COMMAND_HANDLER(handle_md_dar_command)
{
	uint32_t value[256], address, retval;

	if (CMD_ARGC != 1)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], address);

	if(!ap_init) {
		alius_aon_m33_init_ap(global_dap);
		ap_init = 1;
	}

	memset(value, 0, 256 * 4);

	retval = aon_m33_read_memory_1k_by_dar(address, value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read 0x%08x fail", address);
		return retval;
	}

	for(int i = 0; i < 256; i = i + 4)
		command_print(CMD, "0x%08x:  0x%08x 0x%08x 0x%08x 0x%08x", 
			address + 4 * i, value[i], value[i+1], value[i+2], value[i+3]);

	return ERROR_OK;
}

COMMAND_HANDLER(handle_mw_dar_command)
{
	uint32_t value[256], address, retval;

	if (CMD_ARGC != 1)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], address);

	if(!ap_init) {
		alius_aon_m33_init_ap(global_dap);
		ap_init = 1;
	}

	for(int i = 0; i < 256; i++)
		value[i] = i;

	retval = aon_m33_write_memory_1k_by_dar(address, value);
	if(retval != ERROR_OK) {
		command_print(CMD, "write 0x%08x fail", address);
		return retval;
	}

	command_print(CMD, "done");
	return ERROR_OK;
}

COMMAND_HANDLER(handle_md_bd_command)
{
	uint32_t value[4], address, retval;

	if (CMD_ARGC != 1)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], address);

	if(!ap_init) {
		alius_aon_m33_init_ap(global_dap);
		ap_init = 1;
	}

	memset(value, 0, 4 * 4);

	retval = aon_m33_read_memory_16bytes_by_bd(address, value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read 0x%08x fail", address);
		return retval;
	}

	for(int i = 0; i < 4; i = i + 4)
		command_print(CMD, "0x%08x:  0x%08x 0x%08x 0x%08x 0x%08x", 
			address + 4 * i, value[i], value[i+1], value[i+2], value[i+3]);

	return ERROR_OK;
}

COMMAND_HANDLER(handle_mw_bd_command)
{
	uint32_t value[4], address, retval;

	if (CMD_ARGC != 1)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], address);

	if(!ap_init) {
		alius_aon_m33_init_ap(global_dap);
		ap_init = 1;
	}

	for(int i = 0; i < 4; i++)
		value[i] = i;

	retval = aon_m33_write_memory_16bytes_by_bd(address, value);
	if(retval != ERROR_OK) {
		command_print(CMD, "write 0x%08x fail", address);
		return retval;
	}

	command_print(CMD, "done");
	return ERROR_OK;
}

const struct command_registration alius_aon_m33_command_handlers[] = {
	{
		.name = "info",
		.handler = &handle_info_command,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "display m33 info",
	},
	{
		.name = "md",
		.handler = &handle_md_command,
		.mode = COMMAND_ANY,
		.usage = "address",
		.help = "dump memory, u32",
	},
	{
		.name = "mw",
		.handler = &handle_mw_command,
		.mode = COMMAND_ANY,
		.usage = "address value",
		.help = "write memory, u32",
	},
	{
		.name = "md1k",
		.handler = &handle_md_dar_command,
		.mode = COMMAND_ANY,
		.usage = "address",
		.help = "dump 1k memory, u32, use dar register, so make sure address bit[0:9] is 0",
	},
	{
		.name = "mw1k",
		.handler = &handle_mw_dar_command,
		.mode = COMMAND_ANY,
		.usage = "address",
		.help = "write 1k memory, value form 0 - 255, use dar register, so make sure address bit[0:9] is 0",
	},
	{
		.name = "md16",
		.handler = &handle_md_bd_command,
		.mode = COMMAND_ANY,
		.usage = "address",
		.help = "dump 16 bytes memory, u32, use bd register, so make sure address bit[0:3] is 0",
	},
	{
		.name = "mw16",
		.handler = &handle_mw_bd_command,
		.mode = COMMAND_ANY,
		.usage = "address",
		.help = "write 16 bytes memory, u32, use bd register, so make sure address bit[0:3] is 0",
	},
	COMMAND_REGISTRATION_DONE
};
