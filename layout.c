// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#include "eeprom.h"
#include "layout.h"
#include "field.h"
#include "parsing_libjson.h"

#define LAYOUT_CHECK_BYTE	44
#define NO_LAYOUT_FIELDS	"Unknown layout. Dumping raw data\n"
#define ARRAY_LEN(x)		(sizeof(x) / sizeof((x)[0]))

#define LAYOUT_PATH "/etc/eeprom"

struct field layout_unknown[1] = {
	{ NO_LAYOUT_FIELDS, "raw", 256, FIELD_RAW },
};

static int build_layout(struct layout *layout)
{
	char layout_config[32];
	int layout_ver = layout->version;

	// if (layout_ver == LAYOUT_AUTODETECT)
	// 	layout_ver = detect_layout(layout->data);

	sprintf(layout_config, LAYOUT_PATH"/layout-v%d.json", layout_ver);
	return parse_json(layout, layout_config);
}

/*
 * offset_to_string() - convert offset or range to string
 * @dest_str:		A pointer to where the string will be written
 * @offset_start:	The start offset
 * @offset_end:		The end offset
 */
static void offset_to_string(char* dest_str, int offset_start, int offset_end)
{
	ASSERT(dest_str);
	int chars = sprintf(dest_str, "'0x%02x", offset_start);
	if (offset_end != offset_start)
		chars += sprintf(dest_str + chars, "-0x%02x", offset_end);
	sprintf(dest_str + chars, "'");
}

/*
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

struct field *find_field_by_name(struct layout *layout, char *field_name)
{
	struct field *fields = layout->fields;

	for (int i = 0; i < layout->num_of_fields; i++)
		if (fields[i].ops->is_named(&fields[i], field_name))
			return &fields[i];

	ieprintf("Field \"%s\" not found", field_name);

	return NULL;
}

struct field *find_field_by_index(struct layout *layout, int index)
{
	struct field *fields = layout->fields;
	return &fields[index];
}

/*
 * new_layout() - Allocate a new layout based on the data given in buf.
 * @buf:	Data seed for layout
 * @buf_size:	Size of buf
 *
 * Allocates a new layout based on data in buf. The layout version is
 * automatically detected. The resulting layout struct contains a copy of the
 * provided data.
 *
 * Returns: pointer to a new layout on success, NULL on failure
 */
struct layout *new_layout(unsigned char *buf, unsigned int buf_size,
			  int layout_version)
{
	ASSERT(buf);

	struct layout *layout = malloc(sizeof(struct layout));
	if (!layout)
		return NULL;

	layout->version = layout_version;
	layout->data = buf;
	layout->data_size = buf_size;

	if (build_layout(layout) < 0)
		return NULL;

	for (int i = 0; i < layout->num_of_fields; i++) {
		struct field *field = &layout->fields[i];
		field_init(field, buf);
		buf += field->data_size;
	}

	return layout;
}

/*
 * free_layout() - a destructor for layout
 * @layout:	the layout to deallocate
 */
void free_layout(struct layout *layout)
{
	free(layout->fields);
	free(layout);
}
