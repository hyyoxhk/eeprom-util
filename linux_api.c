/*
 * Copyright (C) 2009-2017 CompuLab, Ltd.
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

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include "api.h"
#include "common.h"

extern int errno;

/*
 * Prepares a device file fd for read or write.
 * flags are the standard file open flags.
 * On success: returns the fd.
 * On failure: -1
 */
static int open_device_file(char *dev_file, int i2c_addr)
{
	ASSERT(dev_file);

	int fd = open(dev_file, O_RDWR);
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

static inline __s32 i2c_smbus_access(int file, char read_write, __u8 command,
				     int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;

	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;

	return ioctl(file, I2C_SMBUS, &args);
}

static int i2c_read(struct api *api, unsigned char *buf, int offset, int size)
{
	ASSERT(api && buf);

	int bytes_transferred = 0;
	union i2c_smbus_data data;

	for (int i = offset; i < size; i++) {
		if (i2c_smbus_access(api->fd, I2C_SMBUS_READ, i,
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

static int i2c_write(struct api *api, unsigned char *buf, int offset, int size)
{
	ASSERT(api && buf);

	int bytes_transferred = 0;
	union i2c_smbus_data data;

	for (int i = offset; i < size; i++) {
		data.byte = buf[i];
		if (i2c_smbus_access(api->fd, I2C_SMBUS_WRITE, i,
				I2C_SMBUS_BYTE_DATA, &data) < 0)
			return -1;

		msleep(5);
		bytes_transferred++;
	}

	return bytes_transferred;
}

static int driver_read(struct api *api, unsigned char *buf, int offset,
			int size)
{
	ASSERT(api && buf);
	lseek(api->fd, offset, SEEK_SET);
	return read(api->fd, buf + offset, size);
}

static int driver_write(struct api *api, unsigned char *buf, int offset,
			int size)
{
	ASSERT(api && buf);
	lseek(api->fd, offset, SEEK_SET);
	return write(api->fd, buf + offset, size);
}

static bool i2c_probe(int fd, int addr)
{
	union i2c_smbus_data data;

	if (ioctl(fd, I2C_SLAVE_FORCE, addr) < 0)
		return false;

	if (i2c_smbus_access(fd, I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data) < 0)
		return false;

	return true;
}

#define PRINT_NOT_FOUND(x) eprintf("No "x" was found")
#define PRINT_BUS_NUM(x) (x >= 0) ? eprintf(" on bus %d\n", x) : eprintf("\n")
#define PRINT_DRIVER_HINT(x) eprintf("Is "x" driver loaded?\n")
static int list_i2c_accessible(int bus)
{
	ASSERT(bus <= MAX_I2C_BUS);

	int fd, ret = -1;
	char dev_file_name[20];
	bool i2c_bus_found = false;

	int i = (bus < 0) ? MIN_I2C_BUS : bus;
	int end = (bus < 0) ? MAX_I2C_BUS : bus;
	for (; i <= end; i++) {
		sprintf(dev_file_name, "/dev/i2c-%d", i);

		fd = open(dev_file_name, O_RDWR);
		if (fd < 0 && (errno == ENOENT || errno == ENOTDIR))
			continue;

		i2c_bus_found = true;

		if (fd < 0) {
			eprintf("Failed accessing I2C bus %d: %s (%d)\n",
				i, strerror(errno), -errno);
			continue;
		}

		// return success only if i2c bus exists and can be open
		ret = 0;
		printf("On i2c-%d:\n\t", i);
		for (int j = MIN_I2C_ADDR; j <= MAX_I2C_ADDR; j++)
			if (i2c_probe(fd, j))
				printf("0x%x ", j);

		printf("\n");
		close(fd);
	}

	if (!i2c_bus_found) {
		PRINT_NOT_FOUND("I2C device");
		PRINT_BUS_NUM(bus);
		PRINT_DRIVER_HINT("i2c-dev");
	}

	return ret;
}

#define DRIVER_DEV_PATH "/sys/bus/i2c/devices"
static int list_driver_accessible(int bus)
{
	ASSERT(bus <= MAX_I2C_BUS);

	int ret = -1;
	char *dev_file_format = DRIVER_DEV_PATH"/%d-00%02x/eeprom";
	char dev_file_name[44];
	bool driver_found = false;

	if (access(DRIVER_DEV_PATH, F_OK) < 0) {
		eprintf("Failed accessing path %s: %s (%d)\n",
			DRIVER_DEV_PATH, strerror(errno), -errno);
		return ret;
	}

	int i = (bus < 0) ? MIN_I2C_BUS : bus;
	int end = (bus < 0) ? MAX_I2C_BUS : bus;
	for (; i <= end; i++) {
		for (int j = MIN_I2C_ADDR; j <= MAX_I2C_ADDR; j++) {
			sprintf(dev_file_name, dev_file_format, i, j);
			int res = access(dev_file_name, F_OK);
			if (res < 0 && (errno == ENOENT || errno == ENOTDIR))
				continue;

			driver_found = true;

			if (res < 0) {
				eprintf("Failed accessing device %s: %s (%d)\n",
					dev_file_name, strerror(errno), -errno);
				continue;
			}

			// return success only if device exists and accessible
			ret = 0;

			printf("EEPROM device file found at: %s\n",
				dev_file_name);
		}
	}

	if (!driver_found) {
		PRINT_NOT_FOUND("EEPROM device");
		PRINT_BUS_NUM(bus);
		PRINT_DRIVER_HINT("EEPROM");
	}

	return ret;
}

static int list_accessible(struct api *api)
{
	ASSERT(api);

	int ret1, ret2;

	printf(COLOR_GREEN "I2C buses:\n" COLOR_RESET);
	ret1 = list_i2c_accessible(api->i2c_bus);
	printf(COLOR_GREEN "\nEEPROM device files:\n" COLOR_RESET);
	ret2 = list_driver_accessible(api->i2c_bus);

	return -(ret1 && ret2); /* return -1 only if both fail */
}

static void system_error(const char *message)
{
	perror(message);
}

static int setup_interface(struct api *api)
{
	ASSERT(api);
	ASSERT(api->i2c_bus >= MIN_I2C_BUS && api->i2c_bus <= MAX_I2C_BUS);
	ASSERT(api->i2c_addr >= MIN_I2C_ADDR && api->i2c_addr <= MAX_I2C_ADDR);

	char i2cdev_fname[13];
	char eeprom_dev_fname[40];
	int saved_errno;

	sprintf(i2cdev_fname, "/dev/i2c-%d", api->i2c_bus);
	api->fd = open_device_file(i2cdev_fname, api->i2c_addr);
	if (api->fd >= 0) {
		api->read = i2c_read;
		api->write = i2c_write;
		return 0;
	}

	saved_errno = errno;

	sprintf(eeprom_dev_fname, DRIVER_DEV_PATH"/%d-00%02x/eeprom",
		api->i2c_bus, api->i2c_addr);
	api->fd = open_device_file(eeprom_dev_fname, -1);
	if (api->fd < 0) {
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
		api->read = driver_read;
		api->write = driver_write;
		return 0;
	}

	eprintf("Neither EEPROM driver nor i2c device interface is available");
	eprintf("\n");

	return -1;
}

static int api_read_before_setup(struct api *api, unsigned char *buf,
				 int offset, int size)
{
	ASSERT(api);

	// Setup sets the api->read and api->write function pointers on success
	if (setup_interface(api) == 0)
		return api->read(api, buf, offset, size);

	return -1;
}

static int api_write_before_setup(struct api *api, unsigned char *buf,
				  int offset, int size)
{
	ASSERT(api);

	// Setup sets the api->read and api->write function pointers on success
	if (setup_interface(api) == 0)
		return api->write(api, buf, offset, size);

	return -1;
}

void api_init(struct api *api, int i2c_bus, int i2c_addr)
{
	api->i2c_bus = i2c_bus;
	api->i2c_addr = i2c_addr;

	api->read = api_read_before_setup;
	api->write = api_write_before_setup;
	api->probe = list_accessible;
	api->system_error = system_error;
}
