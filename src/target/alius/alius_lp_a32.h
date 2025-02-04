/* SPDX-License-Identifier: GPL-2.0-or-later */

/***************************************************************************
 *   Copyright (C) 2009 Jinping Wu <wunekky@gmail.com>              *
 ***************************************************************************/

#ifndef OPENOCD_ALIUS_LP_A32_H
#define OPENOCD_ALIUS_LP_A32_H

#define LP_A32_CORE0_DEBUG 0x10000
#define LP_A32_CORE0_CTI 0x20000
#define LP_A32_CORE0_PMU 0x30000
#define LP_A32_CORE0_TRACE 0x40000

/* Debug memory */
#define EDSCR_REGRDY_TIMEOUT 500
#define DEBUG_DTRRX 0x080
#define DEBUG_EDITR 0x084
#define DEBUG_EDSCR 0x088
#define DEBUG_DTRTX 0x08c
#define DEBUG_OSLAR 0x300
#define DEBUG_MIDR 0xd00

/* CTI memory */
/* DDI0487G_a_armv8_arm.pdf H8.7 Cross-trigger interface registers */
#define CTI_CONTROL 0x0000
#define CTI_INTACK 0x0010
#define CTI_CTIAPPPULSE 0x001c
#define CTI_OUTEN0 0X00a0
#define CTI_OUTEN1 0x00a4

/* H5.4 Description and allocation of CTI triggers */
#define CTI_EVENT_DEBUG_REQUEST BIT(0)
#define CTI_EVENT_RESTART_REQUEST BIT(1)

/* H9.2.42 EDSCR, External Debug Status and Control Register */
#define DEBUG_STATUS_MASK 0x3F
#define DEBUG_STATUS_RESTARTING 0x1
#define DEBUG_STATUS_NON_DEBUG 0x2
#define DEBUG_STATUS_BREAK_POINT 0x7
#define DEBUG_STATUS_EXTERNAL_DEBUG_REQUEST 0x13
#define DEBUG_STATUS_NORMAL_HALTING_STEP 0x1B
#define DEBUG_STATUS_EXCLUSIVE_HALTING_STEP 0x1F
#define DEBUG_STATUS_OS_UNLOCK_CATCH 0x23
#define DEBUG_STATUS_RESET_CATCH 0x27
#define DEBUG_STATUS_WATCHPOINT 0x2B
#define DEBUG_STATUS_HLT_INSTRUCTION 0x2F
#define DEBUG_STATUS_SOFTWARE_ACCESS_DEBUG_REG 0x33
#define DEBUG_STATUS_EXCEPTION_CATCH 0x37
#define DEBUG_STATUS_NO_SYNDROME_HALTING_STEP 0x3B

#define EDSCR_HDE BIT(14)
#define EDSCR_ITE BIT(24)
#define EDSCR_MA BIT(20)


/* Load register (Thumb state)
 * rd: destination register
 * rn: base register
 */
#define ARMV4_5_T_LDREX(rd, rn) \
	(((rd) << 12) | (0xf << 8) | 0x0) \
	| (((0xe85 << 4) | rn) << 16)

/* Store register (Thumb state)
 * rt: source register
 * rn: base register
 * rd: flag destination register
 * strex rt, rd, [rn]
 */
#define ARMV4_5_T_STREX(rt, rd, rn) \
	(((rt) << 12) | ((rd) << 8) | 0x0) \
	| (((0xe84 << 4) | rn) << 16)

struct cpu_regs {
    /* general register */
    uint32_t r[13];
};

extern const struct command_registration alius_lp_a32_command_handlers[];
#endif /* OPENOCD_ALIUS_LP_A32_H */
