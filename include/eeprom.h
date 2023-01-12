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

/** visibility attribute */
#if defined(__GNUC__) && __GNUC__ >= 4
#define EEPROM_API __attribute__ ((visibility("default")))
#else
#define EEPROM_API
#endif

struct eeprom {
	struct hal *hal;
};

EEPROM_API int eeprom_init(struct eeprom *eeprom, int i2c_bus, int i2c_addr);

EEPROM_API int eeprom_read(struct eeprom *eeprom, char *field, char *value);

EEPROM_API int eeprom_write(struct eeprom *eeprom, char *field, char *value);

#ifdef __cplusplus
}
#endif

#endif
