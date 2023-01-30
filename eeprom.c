// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#include <stdio.h>
#include "eeprom.h"
#include "layout.h"
#include "field.h"
#include "hal.h"
#include "util.h"

#define EEPROM_SIZE 256

static struct hal hal_api;

struct eeprom *eeprom_open(int i2c_bus, int i2c_addr, int layout_ver)
{
	struct eeprom *eeprom;
	int ret;

	ret = hal_init(&hal_api, i2c_bus, i2c_addr);
	if (ret < 0) {
		perror("hal error");
		return NULL;
	}

	eeprom = zalloc(sizeof(struct eeprom) + EEPROM_SIZE);
	if (!eeprom)
		return NULL;

	eeprom->buffer = (unsigned char *)eeprom + sizeof(struct eeprom);

	hal_read(&hal_api, eeprom->buffer, 0, EEPROM_SIZE);

	eeprom->layout = new_layout(eeprom->buffer, EEPROM_SIZE, layout_ver);
	if (!eeprom->layout) {
		perror("Memory allocation error");
		free(eeprom);
		return NULL;
	}

	return eeprom;
}

int eeprom_read_by_index(struct eeprom *eeprom, int index, char *field_value, size_t size)
{
	struct layout *layout = eeprom->layout;
	struct field *field;

	field = find_field_by_index(layout, index);
	if (!field)
		return -1;

	return field->ops->read(field, field_value, size);
}

int eeprom_read_by_name(struct eeprom *eeprom, char *field_name, char *field_value, size_t size)
{
	struct layout *layout = eeprom->layout;
	struct field *field;
	
	field = find_field_by_name(layout, field_name);
	if (!field)
		return -1;
	return field->ops->read(field, field_value, size);
}

int eeprom_write_by_name(struct eeprom *eeprom, char *field_name, char *field_value)
{
	struct layout *layout = eeprom->layout;
	struct field *field;
	int ret = -1;
	
	field = find_field_by_name(layout, field_name);
	if (!field)
		goto error;
	if (field->ops->write(field, field_value))
		ret = 0;
error:
	return ret;
}

void eeprom_close(struct eeprom *eeprom)
{
	hal_write(&hal_api, eeprom->buffer, 0, EEPROM_SIZE);
	free_layout(eeprom->layout);
	free(eeprom);
}
