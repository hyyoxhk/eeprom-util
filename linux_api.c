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

static int fd;
extern int errno;

/*
 * Prepares a device file fd for read or write.
 * flags are the standard file open flags.
 * On success: returns the fd.
 * On failure: -1
 */
static int open_device_file(char *dev_file, int i2c_addr)
{
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

static int i2c_read(unsigned char *buf, int offset, int size)
{
	int bytes_transferred = 0;
	union i2c_smbus_data data;

	/* Reset the reading pointer of the EEPROM to offset 0 */
	i2c_smbus_access(fd, I2C_SMBUS_WRITE, 0, I2C_SMBUS_BYTE, NULL);
	for (int i = offset; i < size; i++) {
		if (i2c_smbus_access(fd, I2C_SMBUS_READ, i,
				I2C_SMBUS_BYTE, &data) < 0)
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

static int i2c_write(unsigned char *buf, int offset, int size)
{
	int bytes_transferred = 0;
	union i2c_smbus_data data;

	/* Reset the reading pointer of the EEPROM to offset 0 */
	i2c_smbus_access(fd, I2C_SMBUS_WRITE, 0, I2C_SMBUS_BYTE, NULL);
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

static int driver_read(unsigned char *buf, int offset, int size)
{
	lseek(fd, offset, SEEK_SET);
	return read(fd, buf + offset, size);
}

static int driver_write(unsigned char *buf, int offset, int size)
{
	lseek(fd, offset, SEEK_SET);
	return write(fd, buf + offset, size);
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

static int list_i2c_accessible(int bus)
{
	int fd, ret = -1;
	char dev_file_name[13];
	struct stat buf;
	bool i2c_bus_found = false;

	int i = (bus < 0) ? 0 : bus;
	int end = (bus < 0) ? 256 : bus + 1;
	for (; i < end; i++) {
		sprintf(dev_file_name, "/dev/i2c-%d", i);
		if (stat(dev_file_name, &buf) == -1)
			continue;

		i2c_bus_found = true;
		fd = open(dev_file_name, O_RDWR);
		if (fd < 0) {
			fprintf(stderr, "Failed accessing I2C bus "
				"%d: %s (%d)\n", i, strerror(errno), -errno);
		} else {
			/*
			 * only if dev_file_name exists and
			 * can be opened, success is returned
			 */
			ret = 0;
			printf("On i2c-%d:\n\t", i);
			for (int j = 0; j < 128; j++) { /* Assuming 7 bit addresses */
				if (i2c_probe(fd, j))
					printf("0x%x ", j);
			}
		}

		printf("\n");
		close(fd);
	}

	if (!i2c_bus_found)
		printf("No I2C bus was found.\nIs i2c-dev driver loaded?\n");

	return ret;
}

static void system_error(const char *message)
{
	perror(message);
}

static void configure_i2c(struct api *api)
{
	api->read = i2c_read;
	api->write = i2c_write;
	api->probe = list_i2c_accessible;
	api->system_error = system_error;
}

static void configure_driver(struct api *api)
{
	api->read = driver_read;
	api->write = driver_write;
	api->probe = NULL;
	api->system_error = system_error;
}

int setup_interface(struct api *api, int i2c_bus, int i2c_addr)
{
	char i2cdev_fname[13];
	char eeprom_dev_fname[40];
	int saved_errno;

	/* In this case we can still do an i2c probe, so setup for i2c */
	if (i2c_bus < 0 && i2c_addr < 0) {
		configure_i2c(api);
		return 0;
	}

	sprintf(i2cdev_fname, "/dev/i2c-%d", i2c_bus);
	fd = open_device_file(i2cdev_fname, i2c_addr);
	if (fd >= 0) {
		configure_i2c(api);
		return 0;
	}

	saved_errno = errno;

	sprintf(eeprom_dev_fname, "/sys/bus/i2c/devices/%d-00%x/eeprom",
		i2c_bus, i2c_addr);
	fd = open_device_file(eeprom_dev_fname, -1);
	if (fd < 0) {
		/* print error which occurred when opening i2c-dev file */
		fprintf(stderr,	"Error, %s access failed: %s (%d)\n",
			i2cdev_fname, strerror(saved_errno), -saved_errno);
		if (saved_errno == ENOENT)
			fprintf(stderr,	"Is i2c-dev driver loaded?\n");

		/* print error which occurred when opening eeprom-dev file */
		fprintf(stderr,	"Error, %s access failed: %s (%d)\n",
			eeprom_dev_fname, strerror(errno), -errno);
		if (errno == ENOENT)
			fprintf(stderr,	"Is EEPROM driver loaded?\n");
	} else {
		configure_driver(api);
		return 0;
	}

	fprintf(stderr,
		"Neither EEPROM driver nor i2c device interface is available\n");

	return -1;
}
