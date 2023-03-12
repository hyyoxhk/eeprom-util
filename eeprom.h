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

#include <stddef.h>

#define EEPROM_SIZE 256

#define LAYOUT_AUTODETECT -1
#define LAYOUT_LEGACY 0
#define LAYOUT_UNRECOGNIZED 254
#define RAW_DATA 255

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

EEPROM_API int eeprom_write_by_index(struct eeprom *eeprom, int index,
				     char *field_value);

EEPROM_API int eeprom_write_by_name(struct eeprom *eeprom, char *field_name,
				    char *field_value);

EEPROM_API void eeprom_close(struct eeprom *eeprom);


EEPROM_API char *eeprom_get_field_name(struct eeprom *eeprom, int index);

EEPROM_API int eeprom_get_field_num(struct eeprom *eeprom);

EEPROM_API void eeprom_clean_field(struct eeprom *eeprom, char *field_name);

#ifdef __cplusplus
}
#endif

#endif /* _EEPROM_H_ */
