// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#include <stdio.h>
#include "eeprom.h"
#include "layout.h"
#include "field.h"
#include "hal.h"

#define EEPROM_SIZE 256

static struct hal hal_api;
static unsigned char buffer[EEPROM_SIZE];

int eeprom_init(struct eeprom *eeprom, int i2c_bus, int i2c_addr)
{
	struct layout *layout;
	int ret;

	ret = hal_init(&hal_api, i2c_bus, i2c_addr);
	if (ret < 0) {
		perror("hal error");
		return -1;
	}

	layout = new_layout(buffer, EEPROM_SIZE, eeprom->layout_ver, eeprom->read_format);
	if (!layout) {
		perror("Memory allocation error");
		return -1;
	}

	eeprom->layout = layout;

	return 0;
}

static struct field *find_field(struct layout *layout, char *field_name)
{
	struct field *fields = layout->fields;

	for (int i = 0; i < layout->num_of_fields; i++)
		if (fields[i].ops->is_named(&fields[i], field_name))
			return &fields[i];

	ieprintf("Field \"%s\" not found", field_name);

	return NULL;
}

int eeprom_read(struct eeprom *eeprom, char *field_name, char *field_value, size_t size)
{
	struct layout *layout = eeprom->layout;
	struct field *field;
	
	field = find_field(layout, field_name);
	if (!field)
		return 0;
	field->ops->read(field, field_value, field->data_size);

	return 0;
}

int eeprom_write(struct eeprom *eeprom, char *field_name, char *field_value)
{
	struct layout *layout = eeprom->layout;
	struct field *field;
	int ret = -1;
	
	field = find_field(layout, field_name);
	if (!field)
		goto error;
	if (field->ops->write(field, field_value))
		ret = 0;
error:
	return ret;
}

int eeprom_exit(struct eeprom *eeprom)
{
	return 0;
}