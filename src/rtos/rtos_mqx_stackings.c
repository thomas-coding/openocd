// SPDX-License-Identifier: GPL-2.0-or-later

/***************************************************************************
 *   Copyright (C) 2014 by Marian Cingel                                   *
 *   cingel.marian@gmail.com                                               *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "rtos.h"
#include "target/armv7m.h"


/*
 * standard exception stack
 * ( stack base, higher memory address )
 * - xpsr		- 0x48
 * - pc			- 0x44
 * - lr			- 0x40
 * - r12		- 0x3C
 * - r3			- 0x38
 * - r2			- 0x34
 * - r1			- 0x30
 * - r0			- 0x2C
 * extended stack in svc_pending handler
 * - lr			- 0x28
 * - r11		- 0x24
 * - r10		- 0x20
 * - r9			- 0x1C
 * - r8			- 0x18
 * - r7			- 0x14
 * - r6			- 0x10
 * - r5			- 0x0C
 * - r4			- 0x08
 * - BASEPRI	- 0x04
 * - SHPR3		- 0x00 ( contains pend_svc exception priority )
 * ( stack head, lower address, stored in 'task->STACK_PTR' )
 */

static const struct stack_register_offset rtos_mqx_arm_v7m_stack_offsets[ARMV7M_NUM_CORE_REGS] = {
	{ ARMV7M_R0,   0x2C, 32 }, /* r0   */
	{ ARMV7M_R1,   0x30, 32 }, /* r1   */
	{ ARMV7M_R2,   0x34, 32 }, /* r2   */
	{ ARMV7M_R3,   0x38, 32 }, /* r3   */
	{ ARMV7M_R4,   0x08, 32 }, /* r4   */
	{ ARMV7M_R5,   0x0C, 32 }, /* r5   */
	{ ARMV7M_R6,   0x10, 32 }, /* r6   */
	{ ARMV7M_R7,   0x14, 32 }, /* r7   */
	{ ARMV7M_R8,   0x18, 32 }, /* r8   */
	{ ARMV7M_R9,   0x1C, 32 }, /* r9   */
	{ ARMV7M_R10,  0x20, 32 }, /* r10  */
	{ ARMV7M_R11,  0x24, 32 }, /* r11  */
	{ ARMV7M_R12,  0x3C, 32 }, /* r12  */
	{ ARMV7M_R13,    -2, 32 }, /* sp   */
	{ ARMV7M_R14,  0x28, 32 }, /* lr   */
	{ ARMV7M_PC,   0x44, 32 }, /* pc   */
	{ ARMV7M_xPSR, 0x48, 32 }, /* xPSR */
};

const struct rtos_register_stacking rtos_mqx_arm_v7m_stacking = {
	.stack_registers_size = 0x4C,					/* calculate offset base address */
	.stack_growth_direction = -1,
	.num_output_registers = ARMV7M_NUM_CORE_REGS,
	.register_offsets = rtos_mqx_arm_v7m_stack_offsets
};
