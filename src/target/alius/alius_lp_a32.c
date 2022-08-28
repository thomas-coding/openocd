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
#include "target/arm_opcodes.h"

extern struct adiv5_dap *global_dap;
uint32_t lp_a32_init = 0;
int lp_read_a32_core0_debug(uint32_t reg, uint32_t *value);
int lp_write_a32_core0_debug(uint32_t reg, uint32_t value);
int lp_read_a32_core0_cti(uint32_t reg, uint32_t *value);
int lp_write_a32_core0_cti(uint32_t reg, uint32_t value);
int lp_read_reg(struct command_invocation *cmd, uint32_t reg, uint32_t *value);
int lp_write_reg(struct command_invocation *cmd, uint32_t reg, uint32_t value);


int lp_enable_debug(void) {
	uint32_t retval;
	uint32_t reg;

	/* unlock memory map address access */
	retval = lp_write_a32_core0_debug(DEBUG_OSLAR, 0);
	if (retval != ERROR_OK)
		return retval;

	/* enable cti */
	retval = lp_write_a32_core0_cti(CTI_CONTROL, 0x1);
	if (retval != ERROR_OK)
		return retval;

	/* enable halt debug */
	retval = lp_read_a32_core0_debug(DEBUG_EDSCR, &reg);
	if (retval != ERROR_OK)
		return retval;
	reg |= EDSCR_HDE;
	retval = lp_write_a32_core0_debug(DEBUG_EDSCR, reg);
	if (retval != ERROR_OK)
		return retval;

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

struct cpu_regs cortext_regs;
int save_cpu_regs(struct command_invocation *cmd) {
	int retval = ERROR_OK;
	uint32_t temp_value;

	for(int i = 0; i < 13; i++) {
		retval = lp_read_reg(cmd, i, &temp_value);
		if(retval != ERROR_OK) {
			return retval;
		}

		cortext_regs.r[i] = temp_value;
	}
	return retval;
}

int restore_cpu_regs(struct command_invocation *cmd) {
	int retval = ERROR_OK;

	for(int i = 0; i < 13; i++) {
		retval = lp_write_reg(cmd, i, cortext_regs.r[i]);
		if(retval != ERROR_OK) {
			return retval;
		}
	}
	return retval;
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


int lp_read_a32_core0_cti(uint32_t reg, uint32_t *value) {
	int retval = ERROR_OK;

	retval = lp_read_a32_debug(LP_A32_CORE0_CTI + reg, value);

	return retval;
}

int lp_write_a32_core0_cti(uint32_t reg, uint32_t value) {
	int retval = ERROR_OK;

	retval = lp_write_a32_debug(LP_A32_CORE0_CTI + reg, value);

	return retval;
}

void get_status_string(uint32_t status, char *name) {
	switch(status) {
		case DEBUG_STATUS_RESTARTING:
			strcpy(name, "restarting");
			break;
		case DEBUG_STATUS_NON_DEBUG:
			strcpy(name, "non debug, normal running");
			break;
		case DEBUG_STATUS_BREAK_POINT:
			strcpy(name, "break point");
			break;
		case DEBUG_STATUS_EXTERNAL_DEBUG_REQUEST:
			strcpy(name, "external debug request");
			break;
		case DEBUG_STATUS_EXCLUSIVE_HALTING_STEP:
			strcpy(name, "exclusive halting halting step");
			break;
		case DEBUG_STATUS_OS_UNLOCK_CATCH:
			strcpy(name, "os unlock catch");
			break;
		case DEBUG_STATUS_RESET_CATCH:
			strcpy(name, "reset catch");
			break;
		case DEBUG_STATUS_WATCHPOINT:
			strcpy(name, "watchpoint");
			break;
		case DEBUG_STATUS_HLT_INSTRUCTION:
			strcpy(name, "hlt instruction");
			break;
		case DEBUG_STATUS_SOFTWARE_ACCESS_DEBUG_REG:
			strcpy(name, "software access debug register");
			break;
		case DEBUG_STATUS_EXCEPTION_CATCH:
			strcpy(name, "exception catch");
			break;
		case DEBUG_STATUS_NO_SYNDROME_HALTING_STEP:
			strcpy(name, "no syndrome halting setp");
			break;
		default:
			LOG_OUTPUT("unknew status: 0x%x\n", status);
			strcpy(name, "unknow");
			break;
	}
}

uint32_t get_reverse(uint32_t opcode) {
	uint16_t first = opcode & 0x0000ffff;
	uint16_t second = (opcode & 0xffff0000) >> 16;
	uint32_t reverse = (first << 16) | second;

	return reverse;
}

int lp_read_reg(struct command_invocation *cmd, uint32_t reg, uint32_t *value) {
	int retval = ERROR_OK;
	uint32_t opcode, opcode_reverse, temp_value;
	int64_t time;

	/* make sure core is halted */
	retval = command_run_line(CMD_CTX, "alius lp halt");
	if (retval != ERROR_OK)
		return ERROR_FAIL;

	/* generate opcode and reverse for send to EDITR */
	/* MCR p14, 0, rn, c0, c5, 0 */
	opcode = ARMV4_5_MCR(14, 0, reg, 0, 5, 0);
	opcode_reverse = get_reverse(opcode);//(opcode << 16) | (opcode >> 16);
	//LOG_OUTPUT("opcode: 0x%08x  opcode_reverse: 0x%08x\n", opcode, opcode_reverse);

	/* execute instruction */
	retval = lp_write_a32_core0_debug(DEBUG_EDITR, opcode_reverse);
	if (retval != ERROR_OK)
		return ERROR_FAIL;

	/* wait fo instruction exceute complete */
	time = timeval_ms();
	while(1) {

		/* Get the EDSCR from debug interface */
		retval = lp_read_a32_core0_debug(DEBUG_EDSCR, &temp_value);
		if(retval != ERROR_OK) {
			command_print(CMD, "get EDSCR status fail");
			return retval;
		}

		if(temp_value & EDSCR_ITE)//instruction complete
			break;

		if (timeval_ms() > time + EDSCR_REGRDY_TIMEOUT) {
			LOG_OUTPUT("Timeout waiting for instruction execute\n");
			return ERROR_TIMEOUT_REACHED;
		}
	}

	/* read for DTRTX */
	retval = lp_read_a32_core0_debug(DEBUG_DTRTX, value);
	if (retval != ERROR_OK)
		return ERROR_FAIL;

	return retval;
}

int lp_write_reg(struct command_invocation *cmd, uint32_t reg, uint32_t value) {
	int retval = ERROR_OK;
	uint32_t opcode, opcode_reverse, temp_value;
	int64_t time;

	/* make sure core is halted */
	retval = command_run_line(CMD_CTX, "alius lp halt");
	if (retval != ERROR_OK)
		return ERROR_FAIL;

	/* write data to cp14 rx */
	retval = lp_write_a32_core0_debug(DEBUG_DTRRX, value);
	if (retval != ERROR_OK)
		return ERROR_FAIL;

	/* generate opcode and reverse for send to EDITR */
	/* MRC p14, 0, rn, c0, c5, 0 */
	opcode = ARMV4_5_MRC(14, 0, reg, 0, 5, 0);
	opcode_reverse = get_reverse(opcode);
	//LOG_OUTPUT("opcode: 0x%08x  opcode_reverse: 0x%08x\n", opcode, opcode_reverse);

	/* execute instruction */
	retval = lp_write_a32_core0_debug(DEBUG_EDITR, opcode_reverse);
	if (retval != ERROR_OK)
		return ERROR_FAIL;

	/* wait fo instruction exceute complete */
	time = timeval_ms();
	while(1) {

		/* Get the EDSCR from debug interface */
		retval = lp_read_a32_core0_debug(DEBUG_EDSCR, &temp_value);
		if(retval != ERROR_OK) {
			command_print(CMD, "get EDSCR status fail");
			return retval;
		}

		if(temp_value & EDSCR_ITE)//instruction complete
			break;

		if (timeval_ms() > time + EDSCR_REGRDY_TIMEOUT) {
			LOG_OUTPUT("Timeout waiting for instruction execute\n");
			return ERROR_TIMEOUT_REACHED;
		}
	}

	return retval;
}

int lp_dump_regs(struct command_invocation *cmd) {
	uint32_t tmp_value, retval;

	/* dump r0-r12 */
	for(int i = 0; i < 13; i++) {
		retval = lp_read_reg(cmd, i, &tmp_value);
		if(retval != ERROR_OK) {
			return retval;
		}
		command_print(CMD, "r%d	: 0x%08x", i, tmp_value);
	}

	/* dump sp */
	retval = lp_read_reg(cmd, 13, &tmp_value);
	if(retval != ERROR_OK) {
		return retval;
	}
	command_print(CMD, "sp	: 0x%08x", tmp_value);

	/* dump lr */
	retval = lp_read_reg(cmd, 14, &tmp_value);
	if(retval != ERROR_OK) {
		return retval;
	}
	command_print(CMD, "lr	: 0x%08x", tmp_value);

	/* dump pc */
	retval = lp_read_reg(cmd, 15, &tmp_value);
	if(retval != ERROR_OK) {
		return retval;
	}
	command_print(CMD, "pc	: 0x%08x", tmp_value);

	return ERROR_OK;
}

int wait_instruction_execute_complete(void) {
	int retval = ERROR_OK;
	uint32_t temp_value;
	int64_t time;

	/* wait fo instruction exceute complete */
	time = timeval_ms();
	while(1) {

		/* Get the EDSCR from debug interface */
		retval = lp_read_a32_core0_debug(DEBUG_EDSCR, &temp_value);
		if(retval != ERROR_OK) {
			LOG_OUTPUT("get EDSCR status fail\n");
			return retval;
		}

		if(temp_value & EDSCR_ITE)//instruction complete
			break;

		if (timeval_ms() > time + EDSCR_REGRDY_TIMEOUT) {
			LOG_OUTPUT("Timeout waiting for instruction execute\n");
			return ERROR_TIMEOUT_REACHED;
		}
	}

	return retval;
}

int lp_read_memory_by_access_mode(struct command_invocation *cmd, uint32_t address, uint32_t *value, uint32_t count) {
	int retval = ERROR_OK;
	uint32_t opcode, opcode_reverse;
	uint32_t reg;

	/* make sure core is halted */
	retval = command_run_line(CMD_CTX, "alius lp halt");
	if (retval != ERROR_OK)
		return retval;

	/* we will use r0 r1 for read memory, save it */
	retval = save_cpu_regs(cmd);
	if (retval != ERROR_OK)
		return retval;

	/* ------ step 1: write address  to r0 ------ */
	/* write address to cp14 rx */
	retval = lp_write_a32_core0_debug(DEBUG_DTRRX, address);
	if (retval != ERROR_OK)
		return retval;

	/* generate opcode and reverse for send to EDITR */
	/* MRC p14, 0, rn, c0, c5, 0 */
	/* Used r0 store address */
	opcode = ARMV4_5_MRC(14, 0, 0, 0, 5, 0);
	opcode_reverse = get_reverse(opcode);
	//LOG_OUTPUT("opcode: 0x%08x  opcode_reverse: 0x%08x\n", opcode, opcode_reverse);

	/* execute instruction */
	retval = lp_write_a32_core0_debug(DEBUG_EDITR, opcode_reverse);
	if (retval != ERROR_OK)
		return retval;

	/* wait fo instruction exceute complete */
	retval = wait_instruction_execute_complete();
	if (retval != ERROR_OK)
		return retval;

	/* now r0 = address */

	/* ------ step 2: dummy read r0 to cp14 tx ------ */
	/* generate opcode and reverse for send to EDITR */
	/* MCR p14, 0, rn, c0, c5, 0 */
	/* Get value from r0 to cp14 TX */
	opcode = ARMV4_5_MCR(14, 0, 0, 0, 5, 0);
	opcode_reverse = get_reverse(opcode);//(opcode << 16) | (opcode >> 16);
	//LOG_OUTPUT("opcode: 0x%08x  opcode_reverse: 0x%08x\n", opcode, opcode_reverse);

	/* execute instruction */
	retval = lp_write_a32_core0_debug(DEBUG_EDITR, opcode_reverse);
	if (retval != ERROR_OK)
		return retval;

	/* wait fo instruction exceute complete */
	retval = wait_instruction_execute_complete();
	if (retval != ERROR_OK)
		return retval;


	/* ------ step 3: change to memory access mode ------ */
	retval = lp_read_a32_core0_debug(DEBUG_EDSCR, &reg);
	if (retval != ERROR_OK)
		return retval;
	reg |= EDSCR_MA;
	retval = lp_write_a32_core0_debug(DEBUG_EDSCR, reg);
	if (retval != ERROR_OK)
		return retval;

	/* ------ step 4: read DTRTX and discard value ------ */
	/* read from DTRTX */
	retval = lp_read_a32_core0_debug(DEBUG_DTRTX, value);
	if (retval != ERROR_OK)
		return retval;

	/* ------ step 5: read DTRTX , get value ------ */
	/* hardware will do:
	 * LDR R1,[R0],#4
	 * MCR p14,0,R1,c0,c5,0
	 * it will get the [r0] to cp14 DTRTX and loop this */
	count--;
	while(count) {
		retval = lp_read_a32_core0_debug(DEBUG_DTRTX, value);
		if (retval != ERROR_OK)
			return retval;
		count--;
		value++;
	}

	/* ------ step 6: set access mode back to normal mode ------ */
	retval = lp_read_a32_core0_debug(DEBUG_EDSCR, &reg);
	if (retval != ERROR_OK)
		return retval;
	reg &= ~EDSCR_MA;
	retval = lp_write_a32_core0_debug(DEBUG_EDSCR, reg);
	if (retval != ERROR_OK)
		return retval;

	/* ------ step 7: read DTRTX , get last value ------ */
	retval = lp_read_a32_core0_debug(DEBUG_DTRTX, value);
	if (retval != ERROR_OK)
		return retval;

	retval = restore_cpu_regs(cmd);
	if (retval != ERROR_OK)
		return retval;
	return retval;
}

int lp_read_memory(struct command_invocation *cmd, uint32_t address, uint32_t *value) {
	int retval = ERROR_OK;
	uint32_t opcode, opcode_reverse;

	/* make sure core is halted */
	retval = command_run_line(CMD_CTX, "alius lp halt");
	if (retval != ERROR_OK)
		return retval;

	/* we will use r0 r1 for read memory, save it */
	retval = save_cpu_regs(cmd);
	if (retval != ERROR_OK)
		return retval;

	/* write address to cp14 rx */
	retval = lp_write_a32_core0_debug(DEBUG_DTRRX, address);
	if (retval != ERROR_OK)
		return retval;

	/* generate opcode and reverse for send to EDITR */
	/* MRC p14, 0, rn, c0, c5, 0 */
	/* Used r0 store address */
	opcode = ARMV4_5_MRC(14, 0, 0, 0, 5, 0);
	opcode_reverse = get_reverse(opcode);
	//LOG_OUTPUT("opcode: 0x%08x  opcode_reverse: 0x%08x\n", opcode, opcode_reverse);

	/* execute instruction */
	retval = lp_write_a32_core0_debug(DEBUG_EDITR, opcode_reverse);
	if (retval != ERROR_OK)
		return retval;

	/* wait fo instruction exceute complete */
	retval = wait_instruction_execute_complete();
	if (retval != ERROR_OK)
		return retval;

	/* now r0 = address */

	/* generate opcode and reverse for send to EDITR */
	/* LDREX R1, [R0] */
	/* load r0 content to r1 */
	opcode = ARMV4_5_T_LDREX(1, 0);//0xe8501f00;
	opcode_reverse = get_reverse(opcode);
	//LOG_OUTPUT("opcode: 0x%08x  opcode_reverse: 0x%08x\n", opcode, opcode_reverse);

	/* execute instruction */
	retval = lp_write_a32_core0_debug(DEBUG_EDITR, opcode_reverse);
	if (retval != ERROR_OK)
		return retval;

	/* wait fo instruction exceute complete */
	retval = wait_instruction_execute_complete();
	if (retval != ERROR_OK)
		return retval;

	/* now r1 = [address] */

	/* generate opcode and reverse for send to EDITR */
	/* MCR p14, 0, rn, c0, c5, 0 */
	/* Get value from r1 to cp14 TX */
	opcode = ARMV4_5_MCR(14, 0, 1, 0, 5, 0);
	opcode_reverse = get_reverse(opcode);//(opcode << 16) | (opcode >> 16);
	//LOG_OUTPUT("opcode: 0x%08x  opcode_reverse: 0x%08x\n", opcode, opcode_reverse);

	/* execute instruction */
	retval = lp_write_a32_core0_debug(DEBUG_EDITR, opcode_reverse);
	if (retval != ERROR_OK)
		return ERROR_FAIL;

	/* wait fo instruction exceute complete */
	retval = wait_instruction_execute_complete();
	if (retval != ERROR_OK)
		return retval;

	/* now DTRTX = value */
	/* read from DTRTX */
	retval = lp_read_a32_core0_debug(DEBUG_DTRTX, value);
	if (retval != ERROR_OK)
		return ERROR_FAIL;

	retval = restore_cpu_regs(cmd);
	if (retval != ERROR_OK)
		return retval;
	return retval;
}

int lp_write_memory(struct command_invocation *cmd, uint32_t address, uint32_t value) {
	int retval = ERROR_OK;
	uint32_t opcode, opcode_reverse;

	/* make sure core is halted */
	retval = command_run_line(CMD_CTX, "alius lp halt");
	if (retval != ERROR_OK)
		return retval;

	/* we will use r0 r1 r2 for write memory, save it */
	retval = save_cpu_regs(cmd);
	if (retval != ERROR_OK)
		return retval;

	/* write address to cp14 rx */
	retval = lp_write_a32_core0_debug(DEBUG_DTRRX, address);
	if (retval != ERROR_OK)
		return retval;
	//LOG_OUTPUT("DEBUG_DTRRX: 0x%08x\n", address);

	/* generate opcode and reverse for send to EDITR */
	/* MRC p14, 0, rn, c0, c5, 0 */
	/* Used r0 store address */
	opcode = ARMV4_5_MRC(14, 0, 0, 0, 5, 0);
	opcode_reverse = get_reverse(opcode);
	//LOG_OUTPUT("opcode: 0x%08x  opcode_reverse: 0x%08x\n", opcode, opcode_reverse);

	/* execute instruction */
	retval = lp_write_a32_core0_debug(DEBUG_EDITR, opcode_reverse);
	if (retval != ERROR_OK)
		return retval;

	/* wait fo instruction exceute complete */
	retval = wait_instruction_execute_complete();
	if (retval != ERROR_OK)
		return retval;

	/* now r0 = address */

	/* write value to cp14 rx */
	retval = lp_write_a32_core0_debug(DEBUG_DTRRX, value);
	if (retval != ERROR_OK)
		return retval;
	//LOG_OUTPUT("DEBUG_DTRRX: 0x%08x\n", value);

	/* generate opcode and reverse for send to EDITR */
	/* MRC p14, 0, rn, c0, c5, 0 */
	/* Used r0 store address */
	opcode = ARMV4_5_MRC(14, 0, 1, 0, 5, 0);
	opcode_reverse = get_reverse(opcode);
	//LOG_OUTPUT("opcode: 0x%08x  opcode_reverse: 0x%08x\n", opcode, opcode_reverse);

	/* execute instruction */
	retval = lp_write_a32_core0_debug(DEBUG_EDITR, opcode_reverse);
	if (retval != ERROR_OK)
		return retval;

	/* wait fo instruction exceute complete */
	retval = wait_instruction_execute_complete();
	if (retval != ERROR_OK)
		return retval;

	/* now r1 = value */

	/* generate opcode and reverse for send to EDITR */
	/* LDREX R1, [R0] */
	/* load r0 content to r1 */
	opcode = ARMV4_5_T_STREX(1, 2, 0);//0xe8401200;
	opcode_reverse = get_reverse(opcode);
	//LOG_OUTPUT("opcode: 0x%08x  opcode_reverse: 0x%08x\n", opcode, opcode_reverse);

	/* execute instruction */
	retval = lp_write_a32_core0_debug(DEBUG_EDITR, opcode_reverse);
	if (retval != ERROR_OK)
		return retval;

	/* wait fo instruction exceute complete */
	retval = wait_instruction_execute_complete();
	if (retval != ERROR_OK)
		return retval;

	/* now [address] = value */

	retval = restore_cpu_regs(cmd);
	if (retval != ERROR_OK)
		return retval;
	return retval;
}

COMMAND_HANDLER(handle_mw_command)
{
	uint32_t value, address, retval;

	if (CMD_ARGC != 2)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], address);
	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[1], value);

	if(!lp_a32_init) {
		alius_lp_a32_init(global_dap);
		lp_a32_init = 1;
	}

	retval = lp_write_memory(cmd, address, value);
	if(retval != ERROR_OK) {
		command_print(CMD, "write 0x%08x fail", address);
		return retval;
	}

	command_print(CMD, "0x%08x:  0x%08x", address, value);
	return ERROR_OK;
}

COMMAND_HANDLER(handle_md_command)
{
	uint32_t address, retval, count;
	uint32_t *value;

	if (CMD_ARGC != 1 && CMD_ARGC != 2)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], address);

	if(CMD_ARGC == 2)
		COMMAND_PARSE_NUMBER(u32, CMD_ARGV[1], count);
	else
		count = 1;

	value = malloc(4 * count);
	if(!value) {
		command_print(CMD, "read 0x%08x fail, malloc %d fail", address, count);
		return ERROR_FAIL;
	}

	if(!lp_a32_init) {
		alius_lp_a32_init(global_dap);
		lp_a32_init = 1;
	}

	//retval = lp_read_memory(cmd, address, &value);
	retval = lp_read_memory_by_access_mode(cmd, address, value, count);
	if(retval != ERROR_OK) {
		command_print(CMD, "read 0x%08x fail", address);
		return retval;
	}

	for(uint32_t i = 0; i < count; i++) {
		command_print(CMD, "0x%08x:  0x%08x", address, *value);
		address += 4;
		value++;
	}
	return ERROR_OK;
}

COMMAND_HANDLER(handle_regsdump_command)
{
	uint32_t retval;

	if (CMD_ARGC != 0)
		return ERROR_COMMAND_SYNTAX_ERROR;

	if(!lp_a32_init) {
		alius_lp_a32_init(global_dap);
		lp_a32_init = 1;
	}

	retval = lp_dump_regs(CMD);
	if(retval != ERROR_OK) {
		command_print(CMD, "dump core registers fail");
		return retval;
	}

	return ERROR_OK;
}

COMMAND_HANDLER(handle_regr_command)
{
	uint32_t value, reg, retval;

	if (CMD_ARGC != 1)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], reg);

	if(!lp_a32_init) {
		alius_lp_a32_init(global_dap);
		lp_a32_init = 1;
	}

	retval = lp_read_reg(CMD, reg, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read 0x%08x fail", reg);
		return retval;
	}

	command_print(CMD, "0x%03x:  0x%08x", reg, value);
	return ERROR_OK;
}

COMMAND_HANDLER(handle_regw_command)
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

	retval = lp_write_reg(CMD, reg, value);
	if(retval != ERROR_OK) {
		command_print(CMD, "write 0x%08x fail", reg);
		return retval;
	}

	command_print(CMD, "0x%03x:  0x%08x", reg, value);
	return ERROR_OK;
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

	command_print(CMD, "0x%03x:  0x%08x", reg, value);
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

	command_print(CMD, "0x%03x:  0x%08x", reg, value);
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

COMMAND_HANDLER(handle_read_a32_cti_reg_command)
{
	uint32_t value, reg, retval;

	if (CMD_ARGC != 1)
		return ERROR_COMMAND_SYNTAX_ERROR;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[0], reg);

	if(!lp_a32_init) {
		alius_lp_a32_init(global_dap);
		lp_a32_init = 1;
	}

	retval = lp_read_a32_core0_cti(reg, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "read 0x%08x fail", reg);
		return retval;
	}

	command_print(CMD, "0x%04x:  0x%08x", reg, value);
	return ERROR_OK;
}

COMMAND_HANDLER(handle_write_a32_cti_reg_command)
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

	retval = lp_write_a32_core0_cti(reg, value);
	if(retval != ERROR_OK) {
		command_print(CMD, "write 0x%08x fail", reg);
		return retval;
	}

	command_print(CMD, "0x%04x:  0x%08x", reg, value);
	return ERROR_OK;
}

COMMAND_HANDLER(handle_halt_command)
{
	uint32_t retval;

	if (CMD_ARGC != 0)
		return ERROR_COMMAND_SYNTAX_ERROR;

	if(!lp_a32_init) {
		alius_lp_a32_init(global_dap);
		lp_a32_init = 1;
	}

	/* Enable trigger0 event from CTI to PE */
	retval = lp_write_a32_core0_cti(CTI_OUTEN0, 0x1);
	if(retval != ERROR_OK) {
		command_print(CMD, "halt fail");
		return retval;
	}

	/* Trigger event 0 to PE */
	retval = lp_write_a32_core0_cti(CTI_CTIAPPPULSE, CTI_EVENT_DEBUG_REQUEST);
	if(retval != ERROR_OK) {
		command_print(CMD, "halt fail");
		return retval;
	}

	return ERROR_OK;
}

COMMAND_HANDLER(handle_unhalt_command)
{
	uint32_t retval;

	if (CMD_ARGC != 0)
		return ERROR_COMMAND_SYNTAX_ERROR;

	if(!lp_a32_init) {
		alius_lp_a32_init(global_dap);
		lp_a32_init = 1;
	}

	/* Clear event 0 */
	retval = lp_write_a32_core0_cti(CTI_INTACK, 1 << 0);
	if(retval != ERROR_OK) {
		command_print(CMD, "unhalt fail");
		return retval;
	}

	/* Enable trigger1 event from CTI to PE */
	retval = lp_write_a32_core0_cti(CTI_OUTEN1, (1 << 1));
	if(retval != ERROR_OK) {
		command_print(CMD, "unhalt fail");
		return retval;
	}

	/* Trigger event 1 to PE */
	retval = lp_write_a32_core0_cti(CTI_CTIAPPPULSE, CTI_EVENT_RESTART_REQUEST);
	if(retval != ERROR_OK) {
		command_print(CMD, "unhalt fail");
		return retval;
	}

	return ERROR_OK;
}

COMMAND_HANDLER(handle_status_command)
{
	uint32_t retval, value;
	char status[256];

	if (CMD_ARGC != 0)
		return ERROR_COMMAND_SYNTAX_ERROR;

	if(!lp_a32_init) {
		alius_lp_a32_init(global_dap);
		lp_a32_init = 1;
	}

	/* Get the EDSCR from debug interface */
	retval = lp_read_a32_core0_debug(DEBUG_EDSCR, &value);
	if(retval != ERROR_OK) {
		command_print(CMD, "get status fail");
		return retval;
	}

	value &= DEBUG_STATUS_MASK;
	get_status_string(value, status);

	command_print(CMD, "debug status:  %s", status);
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
	{
		.name = "rc0cti",
		.handler = &handle_read_a32_cti_reg_command,
		.mode = COMMAND_ANY,
		.usage = "reg",
		.help = "read cti componet, core0 debug register",
	},
	{
		.name = "wc0cti",
		.handler = &handle_write_a32_cti_reg_command,
		.mode = COMMAND_ANY,
		.usage = "reg value",
		.help = "write cti componet, core0 debug register",
	},
	{
		.name = "halt",
		.handler = &handle_halt_command,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "halt core",
	},
	{
		.name = "unhalt",
		.handler = &handle_unhalt_command,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "unhalt core",
	},
	{
		.name = "status",
		.handler = &handle_status_command,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "dump core debug status",
	},
	{
		.name = "regr",
		.handler = &handle_regr_command,
		.mode = COMMAND_ANY,
		.usage = "reg",
		.help = "read core register, alius lp reg 0",
	},
	{
		.name = "regw",
		.handler = &handle_regw_command,
		.mode = COMMAND_ANY,
		.usage = "reg value",
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
	COMMAND_REGISTRATION_DONE
};
