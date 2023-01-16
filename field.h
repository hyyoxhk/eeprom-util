#ifndef _FIELD_
#define _FIELD_

#include <stdbool.h>
#include <stddef.h>

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

struct field_types {
	enum field_type index;
	char *name;
};

enum read_format {
	FORMAT_DEFAULT,
	FORMAT_DUMP,
};

struct field;

struct field_ops {
	bool (*is_named)(const struct field *field, const char *str);
	int (*read)(const struct field *field, char *str, size_t size);
	int (*read_default)(const struct field *field, char *str, size_t size);
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

void field_init(struct field *field, unsigned char *data,
		enum read_format read_format);

#endif
