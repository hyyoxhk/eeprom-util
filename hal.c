/*
 * Copyright (C) 2009-2017 CompuLab, Ltd.
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
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

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "include/hal.h"

/*
 * Prepares a device file fd for read or write.
 * flags are the standard file open flags.
 * On success: returns the fd.
 * On failure: -1
 */
static int open_device_file(char *dev_file, int i2c_addr)
{
	int fd;

	fd = open(dev_file, O_RDWR);
	if (fd < 0)
		return -1;

	if (i2c_addr >= 0) {
		if (ioctl(fd, I2C_SLAVE_FORCE, i2c_addr) < 0) {
			close(fd);
			return -1;
		}
	}

	return fd;
}

static inline int i2c_smbus_access(int file, char read_write, __u8 command,
				     int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;

	return ioctl(file, I2C_SMBUS, &args);
}

static int i2c_read(int fd, unsigned char *buf, int offset, int size)
{
	int bytes_transferred = 0;
	union i2c_smbus_data data;

	for (int i = offset; i < size; i++) {
		if (i2c_smbus_access(fd, I2C_SMBUS_READ, i,
				I2C_SMBUS_BYTE_DATA, &data) < 0)
			return -1;

		buf[i] = (unsigned char)(data.byte & 0xFF);
		bytes_transferred++;
	}

	return bytes_transferred;
}

/*
 * This function supplies the appropriate delay needed for consecutive writes
 * via i2c to succeed
 */
static void msleep(unsigned int msecs)
{
	struct timespec time = {0, 1000000 * msecs};
	nanosleep(&time, NULL);
}

static int i2c_write(int fd, unsigned char *buf, int offset, int size)
{
	int bytes_transferred = 0;
	union i2c_smbus_data data;

	for (int i = offset; i < size; i++) {
		data.byte = buf[i];
		if (i2c_smbus_access(fd, I2C_SMBUS_WRITE, i,
				I2C_SMBUS_BYTE_DATA, &data) < 0)
			return -1;

		msleep(5);
		bytes_transferred++;
	}

	return bytes_transferred;
}

static int driver_read(int fd, unsigned char *buf, int offset, int size)
{
	lseek(fd, offset, SEEK_SET);
	return read(fd, buf + offset, size);
}

static int driver_write(int fd, unsigned char *buf, int offset, int size)
{
	lseek(fd, offset, SEEK_SET);
	return write(fd, buf + offset, size);
}

#define PRINT_NOT_FOUND(x) eprintf("No "x" was found")
#define PRINT_BUS_NUM(x) (x >= 0) ? eprintf(" on bus %d\n", x) : eprintf("\n")
#define PRINT_DRIVER_HINT(x) eprintf("Is "x" driver loaded?\n")

#define DRIVER_DEV_PATH "/sys/bus/i2c/devices"
int hal_init(struct hal *hal, int i2c_bus, int i2c_addr)
{
	char i2cdev_fname[13];
	char eeprom_dev_fname[40];
	int saved_errno;

	sprintf(i2cdev_fname, "/dev/i2c-%d", i2c_bus);
	hal->fd = open_device_file(i2cdev_fname, i2c_addr);
	if (hal->fd >= 0) {
		hal->read = i2c_read;
		hal->write = i2c_write;
		return 0;
	}

	sprintf(eeprom_dev_fname, DRIVER_DEV_PATH"/%d-00%02x/eeprom", i2c_bus, i2c_addr);
	hal->fd = open_device_file(eeprom_dev_fname, -1);
	if (hal->fd < 0) {
		/* print error which occurred when opening i2c-dev file */
		eprintf("Error, %s access failed: %s (%d)\n",
			i2cdev_fname, strerror(saved_errno), -saved_errno);
		if (saved_errno == ENOENT)
			PRINT_DRIVER_HINT("i2c-dev");

		/* print error which occurred when opening eeprom-dev file */
		eprintf("Error, %s access failed: %s (%d)\n",
			eeprom_dev_fname, strerror(errno), -errno);
		if (errno == ENOENT)
			PRINT_DRIVER_HINT("EEPROM");
	} else {
		hal->read = driver_read;
		hal->write = driver_write;
		return 0;
	}

	eprintf("Neither EEPROM driver nor i2c device interface is available");
	eprintf("\n");

	return -1;
}

int hal_read(struct hal *hal, unsigned char *buf, int off, int size)
{
	return hal->read(hal->fd, buf, off, size);
}

int hal_write(struct hal *hal, unsigned char *buf, int off, int size)
{
	return hal->write(hal->fd, buf, off, size);
}
