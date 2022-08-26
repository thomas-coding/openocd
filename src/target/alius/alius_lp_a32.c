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
#include "alius_lp_a32.h"

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
uint32_t lp_a32_init = 0;
int lp_read_a32_core0_debug(uint32_t reg, uint32_t *value);
int lp_write_a32_core0_debug(uint32_t reg, uint32_t value);


int lp_enable_debug(void) {
	uint32_t retval;

	/* unlock memory map address access */
	retval = lp_write_a32_core0_debug(DEBUG_OSLAR, 0);
	return retval;
}

void alius_lp_a32_init(struct adiv5_dap *dap) {

	uint32_t retval;

	dap->ap[0].ap_num = TOP_BASE;
	dap->ap[2].ap_num = TOP_LP_APBAP;
	dap->adi_version = 6;
	retval = lp_enable_debug();
	if (retval != ERROR_OK)
		LOG_OUTPUT("alius lp a32 init fail\n");
}

int lp_read_a32_debug(uint32_t address, uint32_t *value) {
	int retval = ERROR_OK;

	struct adiv5_ap *ap = dap_get_ap(global_dap, TOP_LP_APBAP);

	//write first ap tar
	retval = dap_queue_ap_write_origin(ap, AP_TAR, LP_A32_APBAP + address);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	//get the value from drw, it will read form second drw
	retval = dap_queue_ap_read_origin(ap, AP_DRW, value);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);
	
	return retval;
}

int lp_write_a32_debug(uint32_t address, uint32_t value) {
	int retval = ERROR_OK;

	struct adiv5_ap *ap = dap_get_ap(global_dap, TOP_LP_APBAP);

	//write first ap tar
	retval = dap_queue_ap_write_origin(ap, AP_TAR, LP_A32_APBAP + address);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	//get the value from drw, it will read form second drw
	retval = dap_queue_ap_write_origin(ap, AP_DRW, value);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);
	
	return retval;
}

int lp_read_a32_core0_debug(uint32_t reg, uint32_t *value) {
	int retval = ERROR_OK;

	retval = lp_read_a32_debug(LP_A32_CORE0_DEBUG + reg, value);
	
	return retval;
}

int lp_write_a32_core0_debug(uint32_t reg, uint32_t value) {
	int retval = ERROR_OK;

	retval = lp_write_a32_debug(LP_A32_CORE0_DEBUG + reg, value);
	
	return retval;
}

COMMAND_HANDLER(handle_read_a32_core0_debug_reg_command)
{
	uint32_t value, reg, retval;

	if (CMD_ARGC != 1)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], reg);

	if(!lp_a32_init) {
		alius_lp_a32_init(global_dap);
		lp_a32_init = 1;
	}

	retval = lp_read_a32_core0_debug(reg, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read 0x%08x fail", reg);
		return retval;
	}

	command_print(CMD, "0x%04x:  0x%08x", reg, value);
	return ERROR_OK;
}

COMMAND_HANDLER(handle_write_a32_core0_debug_reg_command)
{
	uint32_t value, reg, retval;

	if (CMD_ARGC != 2)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], reg);
	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[1], value);

	if(!lp_a32_init) {
		alius_lp_a32_init(global_dap);
		lp_a32_init = 1;
	}

	retval = lp_write_a32_core0_debug(reg, value);
	if(retval != ERROR_OK) {
		command_print(CMD, "write 0x%08x fail", reg);
		return retval;
	}

	command_print(CMD, "0x%04x:  0x%08x", reg, value);
	return ERROR_OK;
}

COMMAND_HANDLER(handle_info_command)
{
	command_print(CMD, "lp a32 info, hello user....");
	return ERROR_OK;
}

COMMAND_HANDLER(handle_midr_command)
{
	uint32_t retval, value;
	retval = lp_read_a32_core0_debug(DEBUG_MIDR, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read midr fail");
		return retval;
	}

	command_print(CMD, "MIDR:  0x%08x", value);
	return ERROR_OK;
}

const struct command_registration alius_lp_a32_command_handlers[] = {
	{
		.name = "info",
		.handler = &handle_info_command,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "display lp a32 info",
	},
	{
		.name = "midr",
		.handler = &handle_midr_command,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "display core midr",
	},
	{
		.name = "rc0debug",
		.handler = &handle_read_a32_core0_debug_reg_command,
		.mode = COMMAND_ANY,
		.usage = "reg",
		.help = "read a32 debug componet, core0 debug register",
	},
	{
		.name = "wc0debug",
		.handler = &handle_write_a32_core0_debug_reg_command,
		.mode = COMMAND_ANY,
		.usage = "reg value",
		.help = "write a32 debug componet, core0 debug register",
	},
	COMMAND_REGISTRATION_DONE
};
