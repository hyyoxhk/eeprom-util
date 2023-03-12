// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../util.h"
#include "../eeprom.h"
#include "command.h"

#define COLOR_RED  "\x1B[31m"
#define COLOR_GREEN  "\x1B[32m"
#define COLOR_RESET  "\033[0m"

#ifdef ENABLE_WRITE
static inline bool write_enabled(void) { return true; }
#else
static inline bool write_enabled(void) { return false; }
#endif

#define VERSION "3.2.0-00001-g464c9c1"
#define BUILD_DATE "16 1月 2023"
#define BUILD_TIME "11:53:19"

#define FORMAT_DEFAULT 0
#define FORMAT_DUMP 1

static void print_banner(void)
{
	char *version = strnlen(VERSION, 20) ? " version " VERSION : "";
	char *date = " (" BUILD_DATE " - " BUILD_TIME ")";

	printf("CompuLab EEPROM utility%s%s\n\n", version, date);
}

static void print_help(void)
{
	print_banner();
	printf("Usage: eeprom-util list [<bus_num>]\n");
	printf("       eeprom-util read [-f <print_format>] [-l <layout_version>] <bus_num> <device_addr>\n");


	if (write_enabled()) {
		printf("       eeprom-util write (fields|bytes) [-l <layout_version>] <bus_num> <device_addr> DATA\n");
		printf("       eeprom-util clear [fields|bytes|all] <bus_num> <device_addr> [DATA]\n");
	}

	printf("       eeprom-util version|-v|--version\n");
	printf("       eeprom-util [help|-h|--help]\n");

	printf("\n"
		"COMMANDS\n"
		"   list 	List device addresses accessible via i2c\n"
		"   read 	Read from EEPROM\n");

	if (write_enabled()) {
		printf("   write	Write to EEPROM. Must specify if writing to 'fields' or 'bytes'\n");
		printf("   clear	Clear EEPROM. Default is 'all'. Other options are clearing 'fields' or 'bytes'.\n");
	}

	printf("   version	Print the version banner and exit\n"
	       "   help		Print this help and exit\n");
	printf("\n"
	       "LAYOUT VERSIONS\n"
	       "   The -l option can be used to force the utility to interpret the EEPROM data using the chosen layout.\n"
	       "   If the -l option is omitted, the utility will auto detect the layout based on the data in the EEPROM.\n"
	       "   The following values can be provided with the -l option:\n"
	       "      auto			use auto-detection to print layout\n"
	       "      legacy, 1, 2, 3, 4	print according to layout version\n"
	       "      raw			print raw data\n");
	printf("\n"
	       "PRINT FORMAT\n"
	       "   The following values can be provided with the -f option:\n"
	       "      default	use the default user friendly output\n"
	       "      dump	dump the data (usable for later input using \"write fields\")\n");

	if (write_enabled()) {
		printf("\n"
			"DATA FORMAT\n"
			"   Some commands require additional data input. The data can be passed inline or from standard input.\n"
			"   Inline and standard input can be mixed. The inline input gets executed before the standard input.\n\n"

			"   The patterns of the data input for each command are listed as follows:\n"
			"      write fields:	[<field_name>=<value> ]*\n"
			"      write bytes: 	[[<offset>,<value>[,<value>]* ]|[<offset>-<offset-end>,<value> ]]*\n"
			"      clear fields:	[<field_name> ]*\n"
			"      clear bytes: 	[<offset>[-<offset-end>] ]*\n\n"

			"   When using inline input:\n"
			"      * Each entry should be separated by a white space.\n"
			"      * Quote marks are needed if spaces exist in an entry.\n\n"

			"   When using standard input:\n"
			"      * Each entry should be on its own line.\n"
			"      * Quote marks are not needed if spaces exist in an entry.\n"
			"      * Comments are supported. Any input from a ';' symbol to the end of the line is ignored.\n\n"

			"   Notes for bytes:\n"
			"      * Offset range is inclusive. Range, non-range and sequence inputs can be mixed.\n"
			"      * Writing a single byte value to an offset:		<offset>,<value>\n"
			"      * Writing a single byte value to all offsets in range:	<offset>-<offset-end>,<value>\n"
			"      * Writing a sequence of bytes starting at an offset:	<offset>,<value1>,<value2>,<value3>...\n\n"

			"   Notes for fields:\n"
			"      * The value input should be in the same pattern like in the output of the read command.\n"
			);

		printf("\n"
			"USAGE EXAMPLE\n"
			"   The input for the following command can be passed inline:\n"
			"      eeprom-util write fields 6 0x20 \"Major Revision=1.20\" \"Production Date=01/Feb/2018\" \\\n"
			"         \"1st MAC Address=01:23:45:67:89:ab\"\n"
			"   or via the standard input:\n"
			"      eeprom-util write fields 6 0x20 < fields_data\n"
			"   Where fields_data is the name of a file containing 3 non empty lines:\n"
			"      Major Revision=1.20\n"
			"      Production Date=01/Feb/2018\n"
			"      1st MAC Address=01:23:45:67:89:ab\n"
			);
	}

	printf("\n");
}

static void message_exit(const char *message)
{
	// ASSERT(message);

	eprintf(COLOR_RED "%s" COLOR_RESET, message);
	print_help();
	exit(1);
}

static void cond_usage_exit(bool cond, const char *message)
{
	if (!cond)
		return;

	message_exit(message);
}

static void usage_exit(void)
{
	print_help();
	exit(0);
}

static enum action parse_action(int argc, char *argv[])
{
	// ASSERT(argv && argc > 0);

	if (!strncmp(argv[0], "list", 4)) {
		return EEPROM_LIST;
	} else if (!strncmp(argv[0], "read", 4)) {
		return EEPROM_READ;
	} else if (write_enabled() && !strncmp(argv[0], "clear", 5)) {
		if (argc > 1 && (!strncmp(argv[1], "fields", 6)))
			return EEPROM_CLEAR_FIELDS;
		if (argc > 1 && (!strncmp(argv[1], "bytes", 5)))
			return EEPROM_CLEAR_BYTES;

		return EEPROM_CLEAR;
	} else if (write_enabled() && !strncmp(argv[0], "write", 5)) {
		if (argc > 1) {
			if (!strncmp(argv[1], "fields", 6)) {
				return EEPROM_WRITE_FIELDS;
			} else if (!strncmp(argv[1], "bytes", 5)) {
				return EEPROM_WRITE_BYTES;
			}
		}
	} else if (!strncmp(argv[0], "help", 4) ||
		!strncmp(argv[0], "-h", 2) ||
		!strncmp(argv[0], "--help", 6)) {
		usage_exit();
	} else if (!strncmp(argv[0], "version", 7) ||
		!strncmp(argv[0], "-v", 2) ||
		!strncmp(argv[0], "--version", 9)) {
		print_banner();
		exit(0);
	}

	message_exit("Unknown function!\n");
	return EEPROM_ACTION_INVALID; //To appease the compiler
}

static int parse_layout_version(char *str)
{
	// ASSERT(str);

	if (!strncmp(str, "legacy", 6))
		return LAYOUT_LEGACY;
	else if (!strncmp(str, "raw", 3))
		return RAW_DATA;
	else if (!strncmp(str, "auto", 4))
		return LAYOUT_AUTODETECT;
	else if(!strncmp(str, "v", 1))
		str++;

	int layout = LAYOUT_UNRECOGNIZED;
	if (strtoi_base(&str, &layout, 10) != STRTOI_STR_END)
		message_exit("Invalid layout version!\n");

	if (layout < LAYOUT_AUTODETECT || layout >= LAYOUT_UNRECOGNIZED)
		message_exit("Unknown layout version!\n");

	return layout;
}

static int parse_print_format(char *str)
{
	// ASSERT(str);

	if (!strncmp(str, "default", 7))
		return FORMAT_DEFAULT;
	else if (!strncmp(str, "dump", 4))
		return FORMAT_DUMP;

	message_exit("Unknown print format!\n");
	return FORMAT_DEFAULT;
}

static int parse_i2c_bus(char *str)
{
	// ASSERT(str);

	int value;
	if (strtoi(&str, &value) != STRTOI_STR_END)
		message_exit("Invalid bus number!\n");

	if (value < MIN_I2C_BUS || value > MAX_I2C_BUS) {
		ieprintf("Bus '%d' is out of range (%d-%d)", value,
			MIN_I2C_BUS, MAX_I2C_BUS);
		exit(1);
	}

	return value;
}

static int parse_i2c_addr(char *str)
{
	ASSERT(str);

	int value;
	if (strtoi(&str, &value) != STRTOI_STR_END)
		message_exit("Invalid address number!\n");

	if (value < MIN_I2C_ADDR || value > MAX_I2C_ADDR) {
		ieprintf("Address '0x%02x' is out of range (0x%02x-0x%02x)",
			value, MIN_I2C_ADDR, MAX_I2C_ADDR);
		exit(1);
	}

	return value;
}

#define NEXT_PARAM(argc, argv)	{(argc)--; (argv)++;}
int main(int argc, char *argv[])
{
	struct command *cmd;
	enum action action = EEPROM_ACTION_INVALID;
	struct options options = {
		.layout_ver	= LAYOUT_AUTODETECT,
		.print_format	= FORMAT_DEFAULT,
	};
	struct data_array data;
	int ret = -1;

	if (argc <= 1)
		usage_exit();

	NEXT_PARAM(argc, argv); // Skip program name
	action = parse_action(argc, argv);
	NEXT_PARAM(argc, argv);
	if (action == EEPROM_LIST && argc == 0)
		goto done;

	// parse_action already took care of parsing the bytes/fields qualifier
	if (action == EEPROM_WRITE_BYTES || action == EEPROM_WRITE_FIELDS ||
	    action == EEPROM_CLEAR_FIELDS || action == EEPROM_CLEAR_BYTES)
		NEXT_PARAM(argc, argv);

	// The "all" qualifier is optional for clear command
	if (action == EEPROM_CLEAR && argc > 0 && !strncmp(argv[0], "all", 3))
		NEXT_PARAM(argc, argv);

	// parse optional parameters
	while (argc > 0 && argv[0][0] == '-') {
		switch (argv[0][1]) {
		case 'l':
			NEXT_PARAM(argc, argv);
			cond_usage_exit(argc < 1, "Missing layout version!\n");
			options.layout_ver = parse_layout_version(argv[0]);
			break;
		case 'f':
			NEXT_PARAM(argc, argv);
			cond_usage_exit(argc < 1, "Missing print format!\n");
			options.print_format = parse_print_format(argv[0]);;
			break;
		default:
			message_exit("Invalid option parameter!\n");
		}

		NEXT_PARAM(argc, argv);
	}

	cond_usage_exit(argc < 1, "Missing I2C bus & address parameters!\n");
	options.i2c_bus = parse_i2c_bus(argv[0]);
	NEXT_PARAM(argc, argv);

	if (action == EEPROM_LIST)
		goto done;

	cond_usage_exit(argc < 1, "Missing I2C address parameter!\n");
	options.i2c_addr = parse_i2c_addr(argv[0]);
	NEXT_PARAM(argc, argv);

	// input = argv;
	// input_size = argc;
	// if (is_stdin && add_lines_from_stdin(&input, &input_size))
	// 	return 1;

done:
	cmd = new_command(action, &options, &data);
	if (!cmd)
		perror(STR_ENO_MEM);
	else
		ret = cmd->execute(cmd);

	free_command(cmd);

	if (action == EEPROM_WRITE_FIELDS)
		free(data.fields_changes);
	else if (action == EEPROM_WRITE_BYTES)
		free(data.bytes_changes);
	else if (action == EEPROM_CLEAR_BYTES)
		free(data.bytes_list);

	return ret ? 1 : 0;
}
