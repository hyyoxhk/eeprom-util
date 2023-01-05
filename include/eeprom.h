// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#ifndef _EEPROM_H_
#define _EEPROM_H_

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

#include "field.h"

/** visibility attribute */
#if defined(__GNUC__) && __GNUC__ >= 4
#define EEPROM_API __attribute__ ((visibility("default")))
#else
#define EEPROM_API
#endif

struct eeprom_api {
	int (*read)(unsigned char *buf, int offset, int size);
	int (*write)(unsigned char *buf, int offset, int size);	
};


EEPROM_API int eeprom_init(struct eeprom_api *api, int i2c_bus, int i2c_addr);

EEPROM_API int eeprom_read(struct layout *layout, char *field, char *value);

EEPROM_API int eeprom_write(struct layout *layout, char *field, char *value);

#ifdef __cplusplus
}
#endif

#endif
