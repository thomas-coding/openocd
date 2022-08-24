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

int aon_m33_write_memory_drw(uint32_t address, uint32_t value) {
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

int aon_m33_read_ahb_ap_register(uint32_t reg, uint32_t *value) {
	int retval = ERROR_OK;

	struct adiv5_ap *ap = dap_get_ap(global_dap, TOP_AON_APBAP);

	//write aon tar. value is which ahb_ap reg we need to access
	retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + reg);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	//get the value
	retval = dap_queue_ap_read(ap, AP_DRW, value);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	return retval;
}

int aon_m33_write_ahb_ap_register(uint32_t reg, uint32_t value) {
	int retval = ERROR_OK;

	struct adiv5_ap *ap = dap_get_ap(global_dap, TOP_AON_APBAP);

	//write aon tar. value is which ahb_ap reg we need to access
	retval = dap_queue_ap_write(ap, AP_TAR, AON_M33_AHBAP + reg);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	//write the value
	retval = dap_queue_ap_write(ap, AP_DRW, value);
	if (retval == ERROR_OK)
		retval = dap_run(ap->dap);

	return retval;
}

int aon_m33_enable_debug(void) {
	uint32_t value, retval;

	retval = aon_m33_read_memory_drw(DCB_DHCSR, &value);
	if(retval != ERROR_OK) {
		return retval;
	}

	value = DBGKEY | C_DEBUGEN;
	retval = aon_m33_write_memory_drw(DCB_DHCSR, value);
	if(retval != ERROR_OK) {
		return retval;
	}

	/* clear ns bit for access secure region */
	retval = aon_m33_read_ahb_ap_register(AP_CSW, &value);
	if(retval != ERROR_OK) {
		return retval;
	}
	value &= (~CSW_HNONSEC);
	retval = aon_m33_write_ahb_ap_register(AP_CSW, value);
	if(retval != ERROR_OK) {
		return retval;
	}

	return ERROR_OK;
}

void alius_aon_m33_init(struct adiv5_dap *dap) {
	uint32_t retval;

	dap->ap[0].ap_num = TOP_BASE;
	dap->ap[1].ap_num = AON_M33_AHBAP;
	dap->adi_version = 6;
	retval = aon_m33_enable_debug();
	if(retval != ERROR_OK) {
		LOG_OUTPUT("NOTICE: init fail\n");
	}
}

int aon_m33_halt(void) {
	uint32_t value, retval;

	retval = aon_m33_read_memory_drw(DCB_DHCSR, &value);
	if(retval != ERROR_OK) {
		return retval;
	}

	value &= ~(0xFFFFul << 16);
	value |= DBGKEY | C_HALT | C_DEBUGEN;
	retval = aon_m33_write_memory_drw(DCB_DHCSR, value);
	if(retval != ERROR_OK) {
		return retval;
	}
	return ERROR_OK;
}

int aon_m33_step(void) {
	uint32_t value, retval;

	retval = aon_m33_read_memory_drw(DCB_DHCSR, &value);
	if(retval != ERROR_OK) {
		return retval;
	}

	/* clear halt and set step, after step, hardware will enter halt again */
	value &= ~((0xFFFFul << 16) | C_HALT);
	value |= DBGKEY | C_STEP | C_DEBUGEN;
	retval = aon_m33_write_memory_drw(DCB_DHCSR, value);
	if(retval != ERROR_OK) {
		return retval;
	}
	return ERROR_OK;
}

int aon_m33_unhalt(void) {
	uint32_t value, retval;

	retval = aon_m33_read_memory_drw(DCB_DHCSR, &value);
	if(retval != ERROR_OK) {
		return retval;
	}

	/* clear debug key and halt */
	value &= ~((0xFFFFul << 16) | C_HALT);
	/* add debug key and enable debug */
	value |= DBGKEY | C_DEBUGEN;

	retval = aon_m33_write_memory_drw(DCB_DHCSR, value);
	if(retval != ERROR_OK) {
		return retval;
	}
	return ERROR_OK;
}

int aon_m33_read_core_reg(uint32_t regsel, uint32_t* value) {
	uint32_t tmp_value, retval;
	int64_t time;

	/* halt for enter debug state to read*/
	retval = aon_m33_halt();
	if(retval != ERROR_OK) {
		return retval;
	}

	/* setup select */
	retval = aon_m33_write_memory_drw(DCB_DCRSR, regsel);
	if(retval != ERROR_OK) {
		return retval;
	}

	time = timeval_ms();
	while(1) {

		retval = aon_m33_read_memory_drw(DCB_DHCSR, &tmp_value);
		if(retval != ERROR_OK) {
			return retval;
		}

		if(tmp_value & S_REGRDY)//data ok
			break;

		if (timeval_ms() > time + DHCSR_S_REGRDY_TIMEOUT) {
			LOG_OUTPUT("Timeout waiting for DCRDR transfer ready");
			return ERROR_TIMEOUT_REACHED;
		}
	}

	retval = aon_m33_read_memory_drw(DCB_DCRDR, value);
	if(retval != ERROR_OK) {
		return retval;
	}

	return ERROR_OK;
}

int aon_m33_write_core_reg(uint32_t regsel, uint32_t value) {
	uint32_t tmp_value, retval;
	int64_t time;

	/* halt for enter debug state to read*/
	retval = aon_m33_halt();
	if(retval != ERROR_OK) {
		return retval;
	}

	/* write value to data register */
	retval = aon_m33_write_memory_drw(DCB_DCRDR, value);
	if(retval != ERROR_OK) {
		return retval;
	}

	/* setup select */
	regsel |= DCRSR_WNR;
	retval = aon_m33_write_memory_drw(DCB_DCRSR, regsel);
	if(retval != ERROR_OK) {
		return retval;
	}

	/* wait for operate done */
	time = timeval_ms();
	while(1) {

		retval = aon_m33_read_memory_drw(DCB_DHCSR, &tmp_value);
		if(retval != ERROR_OK) {
			return retval;
		}

		if(tmp_value & S_REGRDY)//data ok
			break;

		if (timeval_ms() > time + DHCSR_S_REGRDY_TIMEOUT) {
			LOG_OUTPUT("Timeout waiting for DCRDR transfer ready");
			return ERROR_TIMEOUT_REACHED;
		}
	}

	return ERROR_OK;
}

int aon_m33_dump_regs(struct command_invocation *cmd) {
	uint32_t tmp_value, retval;

	/* halt for enter debug state to read*/
	retval = aon_m33_halt();
	if(retval != ERROR_OK) {
		return retval;
	}

	/* dump r0-r12 */
	for(int i = 0; i < 13; i++) {
		retval = aon_m33_read_core_reg(i, &tmp_value);
		if(retval != ERROR_OK) {
			return retval;
		}
		command_print(CMD, "r%d	: 0x%08x", i, tmp_value);
	}

	/* dump sp */
	retval = aon_m33_read_core_reg(13, &tmp_value);
	if(retval != ERROR_OK) {
		return retval;
	}
	command_print(CMD, "sp	: 0x%08x", tmp_value);

	/* dump lr */
	retval = aon_m33_read_core_reg(14, &tmp_value);
	if(retval != ERROR_OK) {
		return retval;
	}
	command_print(CMD, "lr	: 0x%08x", tmp_value);

	/* dump pc */
	retval = aon_m33_read_core_reg(15, &tmp_value);
	if(retval != ERROR_OK) {
		return retval;
	}
	command_print(CMD, "pc	: 0x%08x", tmp_value);

	return ERROR_OK;
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
		alius_aon_m33_init(global_dap);
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
		alius_aon_m33_init(global_dap);
		ap_init = 1;
	}

	retval = aon_m33_write_memory_drw(address, value);
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
		alius_aon_m33_init(global_dap);
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
		alius_aon_m33_init(global_dap);
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
		alius_aon_m33_init(global_dap);
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
		alius_aon_m33_init(global_dap);
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

COMMAND_HANDLER(handle_dump_dcb_command)
{
	uint32_t value, retval;
	if (CMD_ARGC != 0)
		return ERROR_COMMAND_SYNTAX_ERROR;

	if(!ap_init) {
		alius_aon_m33_init(global_dap);
		ap_init = 1;
	}

	/* DHCSR */
	retval = aon_m33_read_memory_drw(DCB_DHCSR, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read fail");
		return retval;
	}
	command_print(CMD, "DCB_DHCSR: 0x%08x", value);

	/* DCRSR */
	retval = aon_m33_read_memory_drw(DCB_DCRSR, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read fail");
		return retval;
	}
	command_print(CMD, "DCB_DCRSR: 0x%08x", value);

	/* DCRDR */
	retval = aon_m33_read_memory_drw(DCB_DCRDR, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read fail");
		return retval;
	}
	command_print(CMD, "DCB_DCRDR: 0x%08x", value);

	/* DCB_DEMCR */
	retval = aon_m33_read_memory_drw(DCB_DEMCR, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read fail");
		return retval;
	}
	command_print(CMD, "DCB_DEMCR: 0x%08x", value);

	/* DCB_DAUTHCTRL */
	retval = aon_m33_read_memory_drw(DCB_DAUTHCTRL, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read fail");
		return retval;
	}
	command_print(CMD, "DCB_DAUTHCTRL: 0x%08x", value);

	/* DCB_DSCSR */
	retval = aon_m33_read_memory_drw(DCB_DSCSR, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read fail");
		return retval;
	}
	command_print(CMD, "DCB_DSCSR: 0x%08x", value);

	return ERROR_OK;
}

COMMAND_HANDLER(handle_halt_command)
{
	uint32_t retval;
	if (CMD_ARGC != 0)
		return ERROR_COMMAND_SYNTAX_ERROR;

	if(!ap_init) {
		alius_aon_m33_init(global_dap);
		ap_init = 1;
	}

	retval = aon_m33_halt();
	if(retval != ERROR_OK) {
		command_print(CMD, "halt core fail");
		return retval;
	}

	command_print(CMD, "done");

	return ERROR_OK;
}

COMMAND_HANDLER(handle_unhalt_command)
{
	uint32_t retval;
	if (CMD_ARGC != 0)
		return ERROR_COMMAND_SYNTAX_ERROR;

	if(!ap_init) {
		alius_aon_m33_init(global_dap);
		ap_init = 1;
	}

	retval = aon_m33_unhalt();
	if(retval != ERROR_OK) {
		command_print(CMD, "unhalt core fail");
		return retval;
	}

	command_print(CMD, "done");

	return ERROR_OK;
}

COMMAND_HANDLER(handle_regr_command)
{
	uint32_t retval, regsel, value;

	if (CMD_ARGC != 1)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], regsel);

	if(!ap_init) {
		alius_aon_m33_init(global_dap);
		ap_init = 1;
	}

	retval = aon_m33_read_core_reg(regsel, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read reg%d fail", regsel);
		return retval;
	}

	command_print(CMD, "r%d : 0x%08x", regsel, value);

	return ERROR_OK;
}

COMMAND_HANDLER(handle_regw_command)
{
	uint32_t retval, regsel, value;

	if (CMD_ARGC != 2)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], regsel);
	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[1], value);

	if(!ap_init) {
		alius_aon_m33_init(global_dap);
		ap_init = 1;
	}

	retval = aon_m33_write_core_reg(regsel, value);
	if(retval != ERROR_OK) {
		command_print(CMD, "write reg%d fail", regsel);
		return retval;
	}

	command_print(CMD, "write r%d : 0x%08x", regsel, value);

	return ERROR_OK;
}

COMMAND_HANDLER(handle_regsdump_command)
{
	uint32_t retval;

	if (CMD_ARGC != 0)
		return ERROR_COMMAND_SYNTAX_ERROR;

	if(!ap_init) {
		alius_aon_m33_init(global_dap);
		ap_init = 1;
	}

	retval = aon_m33_dump_regs(CMD);
	if(retval != ERROR_OK) {
		command_print(CMD, "dump core registers fail");
		return retval;
	}

	return ERROR_OK;
}

COMMAND_HANDLER(handle_ap_reg_read_command)
{
	uint32_t retval, reg, value;

	if (CMD_ARGC != 1)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], reg);

	if(!ap_init) {
		alius_aon_m33_init(global_dap);
		ap_init = 1;
	}

	retval = aon_m33_read_ahb_ap_register(reg, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "apb ap read reg%d fail", reg);
		return retval;
	}

	command_print(CMD, "0x%04x : 0x%08x", reg, value);

	return ERROR_OK;
}

COMMAND_HANDLER(handle_ap_reg_write_command)
{
	uint32_t retval, reg, value;

	if (CMD_ARGC != 2)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], reg);
	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[1], value);

	if(!ap_init) {
		alius_aon_m33_init(global_dap);
		ap_init = 1;
	}

	retval = aon_m33_write_ahb_ap_register(reg, value);
	if(retval != ERROR_OK) {
		command_print(CMD, "apb ap write reg%d fail", reg);
		return retval;
	}

	command_print(CMD, "0x%04x : 0x%08x", reg, value);

	return ERROR_OK;
}

COMMAND_HANDLER(handle_step_command)
{
	uint32_t retval;

	if (CMD_ARGC != 0)
		return ERROR_COMMAND_SYNTAX_ERROR;

	if(!ap_init) {
		alius_aon_m33_init(global_dap);
		ap_init = 1;
	}

	retval = aon_m33_step();
	if(retval != ERROR_OK) {
		command_print(CMD, "step fail");
		return retval;
	}

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
	{
		.name = "dcb",
		.handler = &handle_dump_dcb_command,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "dump debug control block registers",
	},
	{
		.name = "halt",
		.handler = &handle_halt_command,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "halt core, enter debug state",
	},
	{
		.name = "unhalt",
		.handler = &handle_unhalt_command,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "unhalt core, let core run",
	},
	{
		.name = "regr",
		.handler = &handle_regr_command,
		.mode = COMMAND_ANY,
		.usage = "reg_number(0 for r0)",
		.help = "read core register",
	},
	{
		.name = "regw",
		.handler = &handle_regw_command,
		.mode = COMMAND_ANY,
		.usage = "reg_number(0 for r0)",
		.help = "write core register",
	},
	{
		.name = "regsdump",
		.handler = &handle_regsdump_command,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "dump core registers",
	},
	{
		.name = "ap_reg_read",
		.handler = &handle_ap_reg_read_command,
		.mode = COMMAND_ANY,
		.usage = "address",
		.help = "read ahb ap register, address is ahb ap reg offset",
	},
	{
		.name = "ap_reg_write",
		.handler = &handle_ap_reg_write_command,
		.mode = COMMAND_ANY,
		.usage = "address value",
		.help = "write ahb ap register, address is ahb ap reg offset",
	},
	{
		.name = "step",
		.handler = &handle_step_command,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "step, execute one instruction and enter debug state again",
	},
	COMMAND_REGISTRATION_DONE
};
