// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "../eeprom.h"

static void print_eeprom(struct eeprom *eeprom, int format)
{
	// ASSERT(layout && layout->fields);
	int num_of_fields;
	char print_buf[64];
	int i;

	num_of_fields = eeprom_get_field_num(eeprom);

	for (i = 0; i < num_of_fields; i++) {
		memset(print_buf, 0, sizeof(print_buf));
		eeprom_read_by_index(eeprom, i, print_buf, sizeof(print_buf));
		printf("%s\n", print_buf);
	}
}

static int execute_command(struct command *cmd)
{
	// ASSERT(cmd && cmd->action != EEPROM_ACTION_INVALID);

	int ret = -1;
	struct eeprom *eeprom;

	// TODO: list

	// ret = eeprom_init(&eeprom, cmd->opts->i2c_bus, cmd->opts->i2c_addr);
	eeprom = eeprom_open(cmd->opts->i2c_bus, cmd->opts->i2c_addr, cmd->opts->layout_ver);
	if (!eeprom) {
		ret = -1;
		goto done;
	}

	switch(cmd->action) {
	case EEPROM_READ:
		print_eeprom(eeprom, cmd->opts->print_format);
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

done:
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
