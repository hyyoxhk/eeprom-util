// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#ifndef _UTIL_
#define _UTIL_

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <errno.h>

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

#ifdef DEBUG
void failed_assert(const char* func, char *file, int line)
{
	eprintf("Assertion Failed in %s() (%s:%d)\n", func, file, line);
	exit(1);
}
#endif /* ifdef DEBUG */

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
static int strtoi_base(char **str, int *dest, int base)
{
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

static inline int strtoi(char **str, int *dest)
{
	return strtoi_base(str, dest, 0);
}

static inline void *zalloc(size_t size)
{
	return calloc(1, size);
}

#ifdef  __cplusplus
}
#endif

#endif
