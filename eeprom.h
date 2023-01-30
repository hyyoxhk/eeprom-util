// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#ifndef _EEPROM_H_
#define _EEPROM_H_

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

/** visibility attribute */
#if defined(__GNUC__) && __GNUC__ >= 4
#define EEPROM_API __attribute__ ((visibility("default")))
#else
#define EEPROM_API
#endif

struct layout;

struct eeprom {
	struct layout *layout;
	unsigned char *buffer;
};

EEPROM_API struct eeprom *eeprom_open(int i2c_bus, int i2c_addr, int layer_ver);

EEPROM_API int eeprom_read_by_index(struct eeprom *eeprom, int index,
				    char *field_value, size_t size);

EEPROM_API int eeprom_read_by_name(struct eeprom *eeprom, char *field_name,
				   char *field_value, size_t size);

EEPROM_API int eeprom_write_by_name(struct eeprom *eeprom, char *field_name,
				    char *field_value);

EEPROM_API void eeprom_close(struct eeprom *eeprom);

struct field;

EEPROM_API struct field *find_field_by_name(struct layout *layout, char *field_name);

EEPROM_API struct field *find_field_by_index(struct layout *layout, int index);

EEPROM_API char *get_name(struct field *field);

EEPROM_API char *get_index(struct field *field);

#ifdef __cplusplus
}
#endif
#endif
