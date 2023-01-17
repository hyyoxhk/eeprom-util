/*
 * Copyright (C) 2009-2011 CompuLab, Ltd.
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
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#include "common.h"
#include "field.h"

/*
 * strtoi_base - convert to int using the given numerical base and point
 *		 to the first character after the number
 *
 * @str:	A pointer to a string containing an integer number at the
 *		beginning. On success the pointer will point to the first
 *		character after the number.
 * @dest:	A pointer where to save the int result
 * @base:	The numerical base of the characters in the input string.
 * 		If 0 the base is determined by the format.
 *
 * Returns:	STRTOI_STR_END on success and all characters read.
 *		STRTOI_STR_CON on success and additional characters remain.
 *		-ERANGE or -EINVAL on failure
 */
int strtoi_base(char **str, int *dest, int base)
{
	ASSERT(str && *str && dest);

	if (**str == '\0')
		return -EINVAL;

	char *endptr;
	errno = 0;
	int num = strtol(*str, &endptr, base);

	if (errno != 0)
		return -errno;

	if (*str == endptr)
		return -EINVAL;

	*dest = num;
	*str = endptr;

	if (*endptr == 0)
		return STRTOI_STR_END;

	return STRTOI_STR_CON;
}

int strtoi(char **str, int *dest)
{
	return strtoi_base(str, dest, 0);
}

// Macro for printing field's input value error messages
#define iveprintf(str, value, name) \
	ieprintf("Invalid value \"%s\" for field \"%s\" - " str, value, name);

static int __read_bin(const struct field *field, char *delimiter, bool reverse,
			char *str, size_t size)
{
	ASSERT(field && field->data && delimiter);

	int i, n, len = 0;
	char *str1 = str;
	int from = reverse ? field->data_size - 1 : 0;
	int to = reverse ? 0 : field->data_size - 1;

	for (i = from; i != to; reverse ? i-- : i++) {
		n = snprintf(str1, size, "%02x%s", field->data[i], delimiter);
		str1 += n;
		len += n;
	}

	len += snprintf(str1, size, "%02x\n", field->data[i]);
	return len;
}

static int __write_bin(struct field *field, const char *value, bool reverse)
{
	ASSERT(field && field->data && field->name && value);

	int len = strlen(value);
	int i = reverse ? len - 1 : 0;

	/* each two characters in the string are fit in one byte */
	if (len > field->data_size * 2) {
		iveprintf("Value is too long", value, field->name);
		return -1;
	}

	/* pad with zeros */
	memset(field->data, 0, field->data_size);

	/* i - string iterator, j - data iterator */
	for (int j = 0; j < field->data_size; j++) {
		int byte = 0;
		char tmp[3] = { 0, 0, 0 };

		if ((reverse && i < 0) || (!reverse && i >= len))
			break;

		for (int k = 0; k < 2; k++) {
			if (reverse && i == 0) {
				tmp[k] = value[i];
				break;
			}

			tmp[k] = value[reverse ? i - 1 + k : i + k];
		}

		char *str = tmp;
		if (strtoi_base(&str, &byte, 16) < 0 || byte < 0 || byte >> 8) {
			iveprintf("Syntax error", value, field->name);
			return -1;
		}

		field->data[j] = (unsigned char)byte;
		i = reverse ? i - 2 : i + 2;
	}

	return 0;
}



static int __write_bin_delim(struct field *field, char *value, char delimiter)
{
	ASSERT(field && field->data && field->name && value);

	int i, val;
	char *bin = value;

	for (i = 0; i < (field->data_size - 1); i++) {
		if (strtoi_base(&bin, &val, 16) != STRTOI_STR_CON ||
		    *bin != delimiter || val < 0 || val >> 8) {
			iveprintf("Syntax error", value, field->name);
			return -1;
		}

		field->data[i] = (unsigned char)val;
		bin++;
	}

	if (strtoi_base(&bin, &val, 16) != STRTOI_STR_END ||
	    val < 0 || val >> 8) {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	field->data[i] = (unsigned char)val;

	return 0;
}


/**
 * read_bin() - read the value of a field from type "binary" to str
 *
 * Treat the field data as simple binary data, and print it as two digit
 * hexadecimal values.
 * Sample output: 0102030405060708090a
 *
 * @field:	an initialized field to read
 */
static int read_bin(const struct field *field, char *str, size_t size)
{
	return __read_bin(field, "", false, str, size);
}

/**
 * read_bin_raw() - read raw data both in hexadecimal and in ascii format to str
 *
 * @field:	an initialized field to read
 */
//TODO
static int read_bin_raw(const struct field *field, char *str, size_t size)
{
	ASSERT(field && field->data);

	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f"
	       "     0123456789abcdef\n");
	int i, j;

	for (i = 0; i < 256; i += 16) {
		printf("%02x: ", i);
		for (j = 0; j < 16; j++) {
			printf("%02x", field->data[i+j]);
			printf(" ");
		}
		printf("    ");

		for (j = 0; j < 16; j++) {
			if (field->data[i+j] == 0x00 || field->data[i+j] == 0xff)
				printf(".");
			else if (field->data[i+j] < 32 || field->data[i+j] >= 127)
				printf("?");
			else
				printf("%c", field->data[i+j]);
		}
		printf("\n");
	}
	return 0;
}

/**
 * write_bin() - Update field with new data in binary form
 *
 * @field:	an initialized field
 * @value:	a string of values (i.e. "10b234a")
 */
static int write_bin(struct field *field, char *value)
{
	return __write_bin(field, value, false);
}

/**
 * read_bin_rev() - print the value of a field from type "reversed" to str
 *
 * Treat the field data as simple binary data, and print it in reverse order
 * as two digit hexadecimal values.
 *
 * Data in field: 0102030405060708090a
 * Sample output: 0a090807060504030201
 *
 * @field:	an initialized field to print
 */
static int read_bin_rev(const struct field *field, char *str, size_t size)
{
	return __read_bin(field, "", true, str, size);
}

/**
 * write_bin_rev() - Update field with new data in binary form, storing it in reverse
 *
 * This function takes a string of byte values, and stores them
 * in the field in the reverse order. i.e. if the input string was "1234",
 * "3412" will be written to the field.
 *
 * @field:	an initialized field
 * @value:	a string of byte values
 */
static int write_bin_rev(struct field *field, char *value)
{
	return __write_bin(field, value, true);
}

/**
 * read_bin_ver() - print the value of a field from type "version" to str
 *
 * Treat the field data as simple binary data, and print it formatted as a
 * version number (2 digits after decimal point).
 * The field size must be exactly 2 bytes.
 *
 * Sample output: 123.45
 *
 * @field:	an initialized field to read
 */
static int read_bin_ver(const struct field *field, char *str, size_t size)
{
	ASSERT(field && field->data);

	if ((field->data[0] == 0xff) && (field->data[1] == 0xff)) {
		field->data[0] = 0;
		field->data[1] = 0;
	}

	return snprintf(str, size, "%#.2f\n", (field->data[1] << 8 | field->data[0]) / 100.0);
}

/**
 * write_bin_ver() - update a "version field" which contains binary data
 *
 * This function takes a version string in the form of x.y (x and y are both
 * decimal values, y is limited to two digits), translates it to the binary
 * form, then writes it to the field. The field size must be exactly 2 bytes.
 *
 * This function strictly enforces the data syntax, and will not update the
 * field if there's any deviation from it. It also protects from overflow.
 *
 * @field:	an initialized field
 * @value:	a version string
 *
 * Returns 0 on success, -1 on failure.
 */
static int write_bin_ver(struct field *field, char *value)
{
	ASSERT(field && field->data && field->name && value);

	char *version = value;
	int num, remainder;

	if (strtoi(&version, &num) != STRTOI_STR_CON && *version != '.') {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	version++;
	if (strtoi(&version, &remainder) != STRTOI_STR_END) {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	if (num < 0 || remainder < 0) {
		iveprintf("Version must be positive", value, field->name);
		return -1;
	}

	if (remainder > 99) {
		iveprintf("Minor version is 1-2 digits", value, field->name);
		return -1;
	}

	num = num * 100 + remainder;
	if (num >> 16) {
		iveprintf("Version is too big", value, field->name);
		return -1;
	}

	field->data[0] = (unsigned char)num;
	field->data[1] = num >> 8;

	return 0;
}

/**
 * read_mac() - read the value of a field from type "mac" to str
 *
 * Treat the field data as simple binary data, and read it formatted as a MAC address.
 * Sample output: 01:02:03:04:05:06
 *
 * @field:	an initialized field to read
 */
static int read_mac(const struct field *field, char *str, size_t size)
{
	return __read_bin(field, ":", false, str, size);
}

/**
 * write_mac() - Update a mac address field which contains binary data
 *
 * @field:	an initialized field
 * @value:	a colon delimited string of byte values (i.e. "1:02:3:ff")
 */
static int write_mac(struct field *field, char *value)
{
	return __write_bin_delim(field, value, ':');
}

static char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/**
 * read_date() - read the value of a field from type "date" to str
 *
 * Treat the field data as simple binary data, and print it formatted as a date.
 * Sample output: 07/Feb/2014
 * 		  56/BAD/9999
 *
 * @field:	an initialized field to read
 */
// TODO
static int read_date(const struct field *field, char *str, size_t size)
{
	ASSERT(field && field->data);

	printf("%02d/", field->data[0]);
	if (field->data[1] >= 1 && field->data[1] <= 12)
		printf("%s", months[field->data[1] - 1]);
	else
		printf("BAD");

	return snprintf(str, size, "/%d\n", field->data[3] << 8 | field->data[2]);
}

static int validate_date(unsigned char day, unsigned char month,
			unsigned int year)
{
	int days_in_february;

	switch (month) {
	case 0:
	case 2:
	case 4:
	case 6:
	case 7:
	case 9:
	case 11:
		if (day > 31)
			return -1;
		break;
	case 3:
	case 5:
	case 8:
	case 10:
		if (day > 30)
			return -1;
		break;
	case 1:
		days_in_february = 28;
		if (year % 4 == 0) {
			if (year % 100 != 0) {
				days_in_february = 29;
			} else if (year % 400 == 0) {
				days_in_february = 29;
			}
		}

		if (day > days_in_february)
			return -1;

		break;
	default:
		return -1;
	}

	return 0;
}

/**
 * write_date() - update a date field which contains binary data
 *
 * This function takes a date string in the form of x/Mon/y (x and y are both
 * decimal values), translates it to the binary representation, then writes it
 * to the field.
 *
 * This function strictly enforces the data syntax, and will not update the
 * field if there's any deviation from it. It also protects from overflow in the
 * year value, and checks the validity of the date.
 *
 * @field:	an initialized field
 * @value:	a date string
 *
 * Returns 0 on success, -1 on failure.
 */
static int write_date(struct field *field, char *value)
{
	ASSERT(field && field->data && field->name && value);

	char *date = value;
	int day, month, year;

	if (strtoi(&date, &day) != STRTOI_STR_CON || *date != '/') {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	if (day == 0) {
		iveprintf("Invalid day", value, field->name);
		return -1;
	}

	date++;
	if (strlen(date) < 4 || *(date + 3) != '/') {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	for (month = 1; month <= 12; month++)
		if (!strncmp(date, months[month - 1], 3))
			break;

	if (strncmp(date, months[month - 1], 3)) {
		iveprintf("Invalid month", value, field->name);
		return -1;
	}

	date += 4;
	if (strtoi(&date, &year) != STRTOI_STR_END) {
		iveprintf("Syntax error", value, field->name);
		return -1;
	}

	if (validate_date(day, month - 1, year)) {
		iveprintf("Invalid date", value, field->name);
		return -1;
	}

	if (year >> 16) {
		iveprintf("Year overflow", value, field->name);
		return -1;
	}

	field->data[0] = (unsigned char)day;
	field->data[1] = (unsigned char)month;
	field->data[2] = (unsigned char)year;
	field->data[3] = (unsigned char)(year >> 8);

	return 0;
}

/**
 * read_ascii() - read the value of a field from type "ascii" to str
 * @field:	an initialized field to read
 */
static int read_ascii(const struct field *field, char *str1, size_t size)
{
	ASSERT(field && field->data);

	char format[8];
	int *str = (int*)field->data;
	int pattern = *str;
	/* assuming field->data_size is a multiple of 32bit! */
	int block_count = field->data_size / sizeof(int);
	char *read_buf = "";

	/* check if str is trivial (contains only 0's or only 0xff's), if so print nothing */
	for (int i = 0; i < block_count - 1; i++) {
		str++;
		if (*str != pattern || (pattern != 0 && pattern != -1)) {
			read_buf = (char*)field->data;
			break;
		}
	}

	sprintf(format, "%%.%ds\n", field->data_size);
	return snprintf(str1, size, format, read_buf);
}

/**
 * write_ascii() - Update field with new data in ASCII form
 * @field:	an initialized field
 * @value:	the new string data
 *
 * Returns 0 on success, -1 of failure (new string too long).
 */
static int write_ascii(struct field *field, char *value)
{
	ASSERT(field && field->data && field->name && value);

	if (strlen(value) >= field->data_size) {
		iveprintf("Value is too long", value, field->name);
		return -1;
	}

	strncpy((char *)field->data, value, field->data_size - 1);
	field->data[field->data_size - 1] = '\0';

	return 0;
}

/**
 * read_reserved() - read the size of a field from type "reserved" to str
 *
 * Read a notice that the following field_size bytes are reserved.
 *
 * Sample output: (64 bytes)
 *
 * @field:	an initialized field to read
 */
static int read_reserved(const struct field *field, char *str, size_t size)
{
	ASSERT(field);
	return snprintf(str, size, "(%d bytes)\n", field->data_size);
}

/**
 * clear_field() - clear a field
 *
 * A cleared field is defined by having all bytes set to 0xff.
 *
 * @field:	an initialized field to clear
 */
static void clear_field(struct field *field)
{
	ASSERT(field && field->data);
	memset(field->data, 0xff, field->data_size);
}

/**
 * is_named() - check if any of the field's names match the given string
 *
 * @field:	an initialized field to check
 * @str:	the string to check
 *
 * Returns:	true if field's names matches, false otherwise.
 */
static bool is_named(const struct field *field, const char *str)
{
	ASSERT(field && field->name && field->short_name && str);

	if (field->type != FIELD_RESERVED && field->type != FIELD_RAW &&
	    (!strcmp(field->name, str) || !strcmp(field->short_name, str)))
		return true;

	return false;
}

/**
 * read_field() - print the given field using the given string format to str
 *
 * @field:	an initialized field to to read
 * @format:	the string format for printf()
 */
static int read_field(const struct field *field, char *format, char *str, size_t size)
{
	ASSERT(field && field->name && field->ops && format);

	snprintf(str, size, format, field->name);
	return field->ops->read(field, str, size);
}

/**
 * read_default() - read the given field using the default format to str
 *
 * @field:	an initialized field to to read
 */
static int read_default(const struct field *field, char *str, size_t size)
{
	return read_field(field, "%-30s", str, size);
}

/**
 * read_dump() - read the given field using the dump format to str
 *
 * @field:	an initialized field to dump
 */
static int read_dump(const struct field *field, char *str, size_t size)
{
	if (field->type != FIELD_RESERVED)
		return read_field(field, "%s=", str, size);
	return -1;
}

#define OPS_UPDATABLE(type) { \
	.is_named	= is_named, \
	.read		= read_##type, \
	.read_default	= read_default, \
	.write		= write_##type, \
	.clear		= clear_field, \
}

#define OPS_PRINTABLE(type) { \
	.is_named	= is_named, \
	.read		= read_##type, \
	.read_default	= read_default, \
	.write		= NULL, \
	.clear		= NULL, \
}

static struct field_ops field_ops[] = {
	[FIELD_BINARY]		= OPS_UPDATABLE(bin),
	[FIELD_REVERSED]	= OPS_UPDATABLE(bin_rev),
	[FIELD_VERSION]		= OPS_UPDATABLE(bin_ver),
	[FIELD_ASCII]		= OPS_UPDATABLE(ascii),
	[FIELD_MAC]		= OPS_UPDATABLE(mac),
	[FIELD_DATE]		= OPS_UPDATABLE(date),
	[FIELD_RESERVED]	= OPS_PRINTABLE(reserved),
	[FIELD_RAW]		= OPS_PRINTABLE(bin_raw)
};

/**
 * field_init() - init field according to field.type
 *
 * @field:		an initialized field with a known field.type to init
 * @data:		the binary data of the field
 * @format:		the read format of the field
 */
void field_init(struct field *field, unsigned char *data, int format)
{
	ASSERT(field && data);

	field->ops = &field_ops[field->type];
	field->data = data;

	if (format == FORMAT_DUMP)
		field->ops->read_default = read_dump;
}
