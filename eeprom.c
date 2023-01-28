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

	hal_read(&hal_api, buffer, 0, EEPROM_SIZE);

	printf("hyyoxhk %s\n", buffer);

	layout = new_layout(buffer, EEPROM_SIZE, eeprom->layout_ver);
	if (!layout) {
		perror("Memory allocation error");
		return -1;
	}

	eeprom->layout = layout;

	return 0;
}

int eeprom_read_by_index(struct eeprom *eeprom, int index, char *field_value, size_t size)
{
	struct layout *layout = eeprom->layout;
	struct field *fields = layout->fields;
	struct field *field = &fields[index];

	return field->ops->read(field, field_value, size);
}

int eeprom_read_by_name(struct eeprom *eeprom, char *field_name, char *field_value, size_t size)
{
	struct layout *layout = eeprom->layout;
	struct field *field;
	
	field = find_field(layout, field_name);
	if (!field)
		return -1;
	return field->ops->read(field, field_value, size);
}

int eeprom_write_by_name(struct eeprom *eeprom, char *field_name, char *field_value)
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

void eeprom_exit(struct eeprom *eeprom)
{
	hal_write(&hal_api, buffer, 0, EEPROM_SIZE);
	free_layout(eeprom->layout);
}
