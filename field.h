// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#ifndef _FIELD_
#define _FIELD_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

typedef enum {
	FIELD_BINARY,
	FIELD_REVERSED,
	FIELD_VERSION,
	FIELD_ASCII,
	FIELD_MAC,
	FIELD_DATE,
	FIELD_RESERVED,
	FIELD_RAW,
}field_type;




struct field_types {
	field_type index;
	char *name;
};

struct field_ops;

struct field {
	char name[64];
	char short_name[16];
	int data_size;
	field_type type;
	unsigned char *data;
	struct field_ops *ops;
};

struct field_ops {
	bool (*is_named)(const struct field *field, const char *str);
	int (*read)(const struct field *field, char *str, size_t size);
	int (*write)(struct field *field, char *value);
	void (*clear)(struct field *field);
};

void field_init(struct field *field, unsigned char *data);

#ifdef __cplusplus
}
#endif

#endif
