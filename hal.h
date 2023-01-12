/*
 * Copyright (C) 2009-2011 CompuLab, Ltd.
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
