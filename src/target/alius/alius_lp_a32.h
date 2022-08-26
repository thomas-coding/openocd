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
#define DEBUG_MIDR 0x0d00


extern const struct command_registration alius_lp_a32_command_handlers[];
#endif /* OPENOCD_ALIUS_LP_A32_H */
