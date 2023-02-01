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

#include <stddef.h>
#include "util.h"

struct field;
struct layout;

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

#ifdef __cplusplus
}
#endif
#endif
