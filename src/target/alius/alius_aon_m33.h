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

extern const struct command_registration alius_aon_m33_command_handlers[];
#endif /* OPENOCD_ALIUS_AON_M33_H */
