// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#include "util.h"
#include "eeprom.h"
#include "layout.h"
#include "field.h"
#include "libjson.h"

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
