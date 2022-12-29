// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#ifndef _EEPROM_H_
#define _EEPROM_H_

#include "field.h"

/** visibility attribute */
#if defined(__GNUC__) && __GNUC__ >= 4
#define EXPORT __attribute__ ((visibility("default")))
#else
#define EXPORT
#endif

struct eeprom_api {
	int (*read)(unsigned char *buf, int offset, int size);
	int (*write)(unsigned char *buf, int offset, int size);	
};


EXPORT int eeprom_init(struct eeprom_api *api, int i2c_bus, int i2c_addr);

EXPORT int eeprom_read(struct layout *layout, char *field, char *value);

EXPORT int eeprom_write(struct layout *layout, char *field, char *value);

#endif
