#ifndef _FIELD_
#define _FIELD_

#include <stdbool.h>

enum field_type {
	FIELD_BINARY,
	FIELD_REVERSED,
	FIELD_VERSION,
	FIELD_ASCII,
	FIELD_MAC,
	FIELD_DATE,
	FIELD_RESERVED,
	FIELD_RAW,
};

struct field;

struct field_ops {
	int (*get_data_size)(const struct field *field);
	bool (*is_named)(const struct field *field, const char *str);
	void (*read_value)(const struct field *field);
	void (*read)(const struct field *field);
	int (*write)(struct field *field, char *value);
	void (*clear)(struct field *field);
};

struct field {
	char name[64];
	char short_name[16];
	int data_size;
	enum field_type type;
	unsigned char *data;
	struct field_ops *ops;
};

#endif
