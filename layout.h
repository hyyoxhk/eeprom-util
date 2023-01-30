// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#ifndef _LAYOUT_
#define _LAYOUT_

#include "common.h"
// #include "field.h"

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
	int version;
	unsigned char *data;
	int data_size;
};

struct layout *new_layout(unsigned char *buf, unsigned int buf_size,
			  int layout_version);

void free_layout(struct layout *layout);

#endif
