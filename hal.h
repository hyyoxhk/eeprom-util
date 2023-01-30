// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#ifndef HAL_H_
#define HAL_H_

struct hal {
	int fd;
	int (*read)(int fd, unsigned char *buf, int offset, int size);
	int (*write)(int fd, unsigned char *buf, int offset, int size);
};

int hal_init(struct hal *hal, int i2c_bus, int i2c_addr);

int hal_read(struct hal *hal, unsigned char *buf, int off, int size);

int hal_write(struct hal *hal, unsigned char *buf, int off, int size);

#endif
