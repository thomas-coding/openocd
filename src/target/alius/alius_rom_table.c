/* SPDX-License-Identifier: GPL-2.0-or-later */

/***************************************************************************
 *   Copyright (C) 2009 Jinping Wu <wunekky@gmail.com>             *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <helper/log.h>
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

uint32_t ap_index = 0;

void get_ap_name(uint32_t address, enum rom_table_region region, char* name) {
	if(region == TOP) {
		switch(address) {
			case TOP_HP_APBAP:
				strcpy(name, "HP APBAP");
				break;
			case TOP_LP_APBAP:
				strcpy(name, "LP APBAP");
				break;
			case TOP_AON_APBAP:
				strcpy(name, "AON APBAP");
				break;
		}
	} else if(region == AON) {
		switch(address) {
			case AON_M33_AHBAP:
				strcpy(name, "M33 AHBAP");
				break;
		}

	} else if(region == LP) {
		switch(address) {
			case LP_A32_APBAP:
				strcpy(name, "LP AHBAP");
				break;
		}
	}
}

enum rom_table_region get_region(uint32_t base_address) {
	enum rom_table_region region = 0;

	/* Get region */
	if(base_address == TOP_HP_APBAP)
		region = HP;
	else if(base_address == TOP_LP_APBAP)
		region = LP;
	else if(base_address == TOP_AON_APBAP)
		region = AON;
	
	return region;
}

/* Use top ap to access sub rom table */
int dump_sub_rom_table(struct command_invocation *cmd, uint32_t base_address) {
	int retval = ERROR_OK;
	uint32_t value;
	char name[256];

	struct adiv5_ap *ap = dap_get_ap(global_dap, base_address);


	for(int i = 0; i < ROM_TABLE_ENTRY_MAX_MUNBER; i++) {
		//write sub ap tar. value is romtable base address
		retval = dap_queue_ap_write(ap, 0xd04, 4 * i);
		if (retval == ERROR_OK)
			retval = dap_run(ap->dap);
			
		retval = dap_queue_ap_read(ap, 0xd0c, &value);
		if (retval == ERROR_OK)
			retval = dap_run(ap->dap);

		/* Check if rom table present */
		if((value & 0x3) == ROM_TABLE_ENTRY_NOT_PRESENT_AND_LAST)
			break;

		memset(name, 0, 256);
		get_ap_name(value & 0xfffffffcUL, get_region(base_address), name);

		command_print(CMD, "	sub rom table enrty[%d]:	0x%08x	%s", i, value, name);
	}

	return retval;
}

int dump_top_rom_table(struct command_invocation *cmd, uint32_t base_address) {
	int retval = ERROR_OK;
	uint32_t value;
	char name[256];


	for(int i = 0; i < ROM_TABLE_ENTRY_MAX_MUNBER; i++) {

		struct adiv5_ap *ap = dap_get_ap(global_dap, base_address);
		//LOG_OUTPUT("[%d]rom table read address:0x%08lx, reg:0x%08x\n",recursion, ap->ap_num, i * 4);
		retval = dap_queue_ap_read(ap, 4 * i, &value);
		if (retval == ERROR_OK)
			retval = dap_run(ap->dap);

		/* Check if rom table present */
		if((value & 0x3) == ROM_TABLE_ENTRY_NOT_PRESENT_AND_LAST)
			break;

		/* Parse entry */
		/* Register ap to DAP */
		global_dap->ap[ap_index].ap_num = value & 0xfffffffcUL;
		struct adiv5_ap *sub_ap = dap_get_ap(global_dap, value & 0xfffffffcUL);
		ap_index++;

		memset(name, 0, 256);
		get_ap_name(sub_ap->ap_num, TOP, name);
		command_print(CMD, "top rom table enrty[%d]:	0x%08x	%s", i, value, name);

		/* Check if need parse sub */
		retval = dap_queue_ap_read(sub_ap, DEV_ARCH_OFFSET, &value);
		if (retval == ERROR_OK)
			retval = dap_run(sub_ap->dap);

		if(value == 0x47700a17) { //sub ap, need parse sub rom table
			dump_sub_rom_table(cmd, sub_ap->ap_num);
		}
	}

	return retval;
}

void alius_init_ap(struct adiv5_dap *dap) {
	dap->ap[0].ap_num = TOP_BASE;
	dap->adi_version = 6;
}

COMMAND_HANDLER(handle_top_command)
{
	alius_init_ap(global_dap);
	dump_top_rom_table(CMD, TOP_BASE);
	return ERROR_OK;
}

const struct command_registration alius_rom_table_command_handlers[] = {
	{
		.name = "top",
		.handler = &handle_top_command,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "parse top level rom table",
	},
	COMMAND_REGISTRATION_DONE
};
