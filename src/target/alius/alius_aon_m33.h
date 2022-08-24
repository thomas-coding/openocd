/* SPDX-License-Identifier: GPL-2.0-or-later */

/***************************************************************************
 *   Copyright (C) 2009 Jinping Wu <wunekky@gmail.com>              *
 ***************************************************************************/

#ifndef OPENOCD_ALIUS_AON_M33_H
#define OPENOCD_ALIUS_AON_M33_H

#define AP_DAR0 0x0000
#define AP_CSW 0x0D00
#define AP_TAR 0x0D04
#define AP_DRW 0x0D0C
#define AP_BD0 0x0D10

/* ns bit */
#define CSW_HNONSEC BIT(30)

/* DCB_DHCSR bit and field definitions */
#define DBGKEY		(0xA05Ful << 16)
#define C_DEBUGEN	BIT(0)
#define C_HALT		BIT(1)
#define C_STEP		BIT(2)
#define C_MASKINTS	BIT(3)
#define S_REGRDY	BIT(16)
#define S_HALT		BIT(17)
#define S_SLEEP		BIT(18)
#define S_LOCKUP	BIT(19)
#define S_RETIRE_ST	BIT(24)
#define S_RESET_ST	BIT(25)

//Debug Control Block
#define DCB_DHCSR 0xE000EDF0  /*Debug Halting Control and Status Register*/
#define DCB_DCRSR 0xE000EDF4  /*Debug Core Register Select Register*/
#define DCB_DCRDR 0xE000EDF8  /*Debug Core Register Data Register*/
#define DCB_DEMCR 0xE000EDFC  /*Debug Exception and Monitor Control Register*/
#define DCB_DAUTHCTRL 0xE000EE04  /*Debug Authentication Control Register*/
#define DCB_DSCSR 0xE000EE08  /*Debug Security Control and Status Register*/

#define DAUTHSTATUS 0xE000EFB8  /*Debug Authentication Status Register*/

#define DCRSR_WNR	BIT(16)

#define DHCSR_S_REGRDY_TIMEOUT (500)

extern const struct command_registration alius_aon_m33_command_handlers[];
#endif /* OPENOCD_ALIUS_AON_M33_H */
