// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#ifndef _LAYOUT_
#define _LAYOUT_

#include "field.h"

struct layout {
	struct field *fields;
	int num_of_fields;
	int version;
	unsigned char *data;
	int data_size;
};

struct layout *new_layout(unsigned char *buf, unsigned int buf_size,
			  int layout_version);

void free_layout(struct layout *layout);

struct field *find_field_by_name(struct layout *layout, char *field_name);

struct field *find_field_by_index(struct layout *layout, int index);


#endif
