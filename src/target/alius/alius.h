/* SPDX-License-Identifier: GPL-2.0-or-later */

/***************************************************************************
 *   Copyright (C) 2009 Jinping Wu <wunekky@gmail.com>              *
 ***************************************************************************/

#ifndef OPENOCD_ALIUS_H
#define OPENOCD_ALIUS_H

struct command_registration;

#define ALIUS_VERSION 1

/**
 * Export the registration for the alius command group, so it can be
 * embedded in example drivers.
 */
extern const struct command_registration alius_command_handlers[];
int alius_register_commands(struct command_context *cmd_ctx);

#endif /* OPENOCD_ALIUS_H */
