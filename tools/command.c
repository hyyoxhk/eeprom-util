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
		
		printf("%-30s", eeprom_get_field_name(eeprom, i));
		eeprom_read_by_index(eeprom, i, print_buf, sizeof(print_buf));
		printf("%s", print_buf);
	}
}

/**
 * offset_to_string() - convert offset or range to string
 * @dest_str:		A pointer to where the string will be written
 * @offset_start:	The start offset
 * @offset_end:		The end offset
 */
static void offset_to_string(char *dest_str, int offset_start, int offset_end)
{
	ASSERT(dest_str);
	int chars = sprintf(dest_str, "'0x%02x", offset_start);
	if (offset_end != offset_start)
		chars += sprintf(dest_str + chars, "-0x%02x", offset_end);
	sprintf(dest_str + chars, "'");
}

/**
 * get_bytes_range() - Test offsets values and return range
 * @offset_start:	The start offset
 * @offset_end:		The end offset
 *
 * Returns: range on success, 0 on failure.
 */
static size_t get_bytes_range(int offset_start, int offset_end)
{
	if (offset_start < 0 || offset_start >= EEPROM_SIZE ||
	    offset_end < offset_start || offset_end >= EEPROM_SIZE) {
		char offset_str[30];
		offset_to_string(offset_str, offset_start, offset_end);
		ieprintf("Invalid offset %s", offset_str);
		return 0;
	}

	return offset_end - offset_start + 1;
}

static int update_eeprom_fields(struct eeprom *eeprom, struct data_array *data)
{
	int updated_fields_cnt = 0;

	for (int i = 0; i < data->size; i++) {
		char *field_name = data->fields_changes[i].field;
		char *field_value = data->fields_changes[i].value;

		if (*field_value == '\0')
			eeprom_clean_field(eeprom, field_name);
		else if (eeprom_write_by_name(eeprom, field_name, field_value))
			return 0;
		updated_fields_cnt++;
	}

	return updated_fields_cnt;

}

static int update_eeprom_bytes(struct eeprom *eeprom, struct data_array *data)
{
	int updated_bytes = 0;

	for (int i = 0; i < data->size; i++) {
		int offset_start = data->bytes_changes[i].start;
		int offset_end = data->bytes_changes[i].end;
		size_t range = get_bytes_range(offset_start, offset_end);
		if (range == 0)
			return 0;

		int value = data->bytes_changes[i].value;
		if (value >= 0 && value <= 255){
			memset(eeprom->buffer + offset_start, value, range);
			updated_bytes += range;
			continue;
		}

		char value_str[60];
		int chars = sprintf(value_str, "'0x%02x' at offset ", value);
		offset_to_string(value_str + chars, offset_start, offset_end);
		ieprintf("Invalid value %s", value_str);
		return 0;
	}

	return updated_bytes;
}

static int clear_eeprom_fields(struct eeprom *eeprom, struct data_array *data)
{
	int cleared_fields_cnt = 0;

	for (int i = 0; i < data->size; i++) {
		char *field_name = data->fields_changes[i].field;
		eeprom_clean_field(eeprom, field_name);
		cleared_fields_cnt++;
	}

	return cleared_fields_cnt;
}

static int clear_eeprom_bytes(struct eeprom *eeprom, struct data_array *data)
{
	int cleared_bytes = 0;

	for (int i = 0; i < data->size; i++) {
		int offset_start = data->bytes_list[i].start;
		int offset_end = data->bytes_list[i].end;
		size_t range = get_bytes_range(offset_start, offset_end);
		if (range == 0)
			return 0;

		memset(eeprom->buffer + offset_start, 0xff, range);
		cleared_bytes += range;
	}

	return cleared_bytes;
}

static int execute_command(struct command *cmd)
{
	// ASSERT(cmd && cmd->action != EEPROM_ACTION_INVALID);

	int ret = -1;
	struct eeprom *eeprom;

	// TODO: list
	if (cmd->action == EEPROM_LIST) {
		printf("Not support lsit\n");
		return 0;
	}

	eeprom = eeprom_open(cmd->opts->i2c_bus, cmd->opts->i2c_addr, cmd->opts->layout_ver);
	if (!eeprom) {
		return -1;
	}

	switch(cmd->action) {
	case EEPROM_READ:
		print_eeprom(eeprom, cmd->opts->print_format);
		ret = 0;
		goto done;
	case EEPROM_WRITE_FIELDS:
		if (!update_eeprom_fields(eeprom, cmd->data)) {
			ret = -1;
			goto done;
		}
		break;
	case EEPROM_WRITE_BYTES:
		if (!update_eeprom_bytes(eeprom, cmd->data)) {
			ret = -1;
			goto done;
		}
		break;
	case EEPROM_CLEAR_FIELDS:
		if (!clear_eeprom_fields(eeprom, cmd->data)) {
			ret = -1;
			goto done;
		}
		break;
	case EEPROM_CLEAR_BYTES:
		if (!clear_eeprom_bytes(eeprom, cmd->data)) {
			ret = -1;
			goto done;
		}
		break;
	default:
		goto done;
	}

done:
	eeprom_close(eeprom);
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
