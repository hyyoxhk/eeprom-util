/*
 * Copyright (C) 2009-2017 CompuLab, Ltd.
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
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

#ifndef _LAYOUT_
#define _LAYOUT_

#include "common.h"
#include "field.h"

#define EEPROM_SIZE 256

#define LAYOUT_AUTODETECT -1
#define LAYOUT_LEGACY 0

#define LAYOUT_VER1 1
#define LAYOUT_VER2 2
#define LAYOUT_VER3 3
#define LAYOUT_VER4 4

#define LAYOUT_UNRECOGNIZED 254
#define RAW_DATA 255

struct layout {
	struct field *fields;
	int num_of_fields;
	int layout_version;
	unsigned char *data;
	int data_size;
	void (*print)(const struct layout *layout);
	int (*update_fields)(struct layout *layout,
			     struct data_array *data);
	int (*clear_fields)(struct layout *layout,
			    struct data_array *data);
	int (*update_bytes)(struct layout *layout,
			    struct data_array *data);
	int (*clear_bytes)(struct layout *layout,
			   struct data_array *data);
};

struct layout *new_layout(unsigned char *buf, unsigned int buf_size,
			  int layout_version,
			  enum read_format read_format);

void free_layout(struct layout *layout);

struct field *find_field(struct layout *layout, char *field_name);

#endif