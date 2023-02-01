// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#ifndef _UTIL_
#define _UTIL_

#ifdef  __cplusplus
extern "C" {
#endif

/** visibility attribute */
#if defined(__GNUC__) && __GNUC__ >= 4
#define EEPROM_API __attribute__ ((visibility("default")))
#else
#define EEPROM_API
#endif

#include <stdlib.h>

// Macro for handling debug checks
#ifndef DEBUG
#define ASSERT(args) ((void)0)
#else // ifndef DEBUG
void failed_assert(const char* func, char *file, int line);
#define ASSERT(arg) ((arg) ? ((void)0) : \
	failed_assert(__func__, __FILE__, __LINE__))
#endif // ifndef DEBUG

// Macro for printing error messages
#define eprintf(args...) fprintf (stderr, args)
// Macro for printing input error messages
#define ie_fmt(fmt) "Input error: " fmt " - Operation Aborted!\n"
#define ieprintf(fmt, ...) eprintf(ie_fmt(fmt), ##__VA_ARGS__)

#define STRTOI_STR_CON 1
#define STRTOI_STR_END 2

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
EEPROM_API int strtoi_base(char **str, int *dest, int base);

EEPROM_API int strtoi(char **str, int *dest);

#define zalloc(size) calloc(1, size)

#ifdef  __cplusplus
}
#endif

#endif
