/*
 * Copyright (C) 2009-2011 CompuLab, Ltd.
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>

#include "command.h"
#include "../eeprom.h"

static void print_eeprom(struct eeprom *eeprom, int format)
{
	ASSERT(layout && layout->fields);

	struct layout *layout = eeprom->layout;
	char print_buf[64];
	int i;

	for (i = 0; i < layout->num_of_fields; i++) {
		eeprom_read_by_index(eeprom, i, print_buf, sizeof(print_buf));
		// printf("%s\n", print_buf);
	}
	printf("i = %d\n", i);
}

static int execute_command(struct command *cmd)
{
	ASSERT(cmd && cmd->action != EEPROM_ACTION_INVALID);

	int ret = -1;
	struct eeprom eeprom;

	// TODO: list

	eeprom.layout_ver = cmd->opts->layout_ver;
	eeprom.read_format = cmd->opts->print_format;
	eeprom_init(&eeprom, cmd->opts->i2c_bus, cmd->opts->i2c_addr);


	switch(cmd->action) {
	case EEPROM_READ:
		print_eeprom(&eeprom, cmd->opts->print_format);
		ret = 0;
	case EEPROM_WRITE_FIELDS:
		break;
	case EEPROM_WRITE_BYTES:
		break;
	case EEPROM_CLEAR_FIELDS:

		break;
	case EEPROM_CLEAR_BYTES:

		break;
	default:
	}

	return ret;
}

struct command *new_command(enum action action, struct options *options,
		struct data_array *data)
{
	struct command *cmd = malloc(sizeof(struct command));
	if (!cmd)
		return cmd;

	cmd->action = action;
	cmd->opts = options;
	cmd->data = data;
	cmd->execute = execute_command;

	return cmd;
}

void free_command(struct command *cmd)
{
	free(cmd);
}
