// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#ifndef _COMMAND_
#define _COMMAND_

#include "../layout.h"

enum action {
	EEPROM_READ,
	EEPROM_WRITE_FIELDS,
	EEPROM_WRITE_BYTES,
	EEPROM_LIST,
	EEPROM_CLEAR,
	EEPROM_CLEAR_FIELDS,
	EEPROM_CLEAR_BYTES,
	EEPROM_ACTION_INVALID,
};

struct options {
	int i2c_bus;
	int i2c_addr;
	int layout_ver;
	int print_format;
};

struct command {
	enum action action;
	struct options *opts;
	struct data_array *data;

	int (*execute)(struct command *cmd);
};

struct command *new_command(enum action action, struct options *options,
			    struct data_array *data);
void free_command(struct command *cmd);

#endif
