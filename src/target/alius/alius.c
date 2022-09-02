/* SPDX-License-Identifier: GPL-2.0-or-later */

/***************************************************************************
 *   Copyright (C) 2009 Jinping Wu <wunekky@gmail.com>             *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <helper/log.h>
#include "alius.h"
#include "alius_rom_table.h"
#include "alius_aon_m33.h"
#include "alius_lp_a32.h"

//#define ALIUS_DEBUG

COMMAND_HANDLER(handle_version_command)
{
	command_print(CMD, "Alius command version: %d.%d", ALIUS_VERSION_MAJOR, ALIUS_VERSION_MIMOR);
	return ERROR_OK;
}

static const struct command_registration alius_sub_command_handlers[] = {
	{
		.name = "version",
		.handler = &handle_version_command,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "show the version of alius command",
	},
#ifdef ALIUS_DEBUG
	{
		.name = "romtable",
		.chain = alius_rom_table_command_handlers,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "prase rom table",
	},
	{
		.name = "m33",
		.chain = alius_aon_m33_command_handlers,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "handle aon m33",
	},
	{
		.name = "lp",
		.chain = alius_lp_a32_command_handlers,
		.mode = COMMAND_ANY,
		.usage = "",
		.help = "handle lp a32",
	},
#endif
	COMMAND_REGISTRATION_DONE
};

const struct command_registration alius_command_handlers[] = {
	{
		.name = "alius",
		.mode = COMMAND_ANY,
		.help = "alius cmd for aon, lp, hp",
		.chain = alius_sub_command_handlers,
		.usage = "",
	},
	COMMAND_REGISTRATION_DONE
};

int alius_register_commands(struct command_context *cmd_ctx)
{
	return register_commands(cmd_ctx, NULL, alius_command_handlers);
}
