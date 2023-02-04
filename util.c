// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#include <stdlib.h>
#include <errno.h>

#include "util.h"

#ifdef DEBUG
void failed_assert(const char* func, char *file, int line)
{
	eprintf("Assertion Failed in %s() (%s:%d)\n", func, file, line);
	exit(1);
}
#endif /* DEBUG */

int strtoi_base(char **str, int *dest, int base)
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

int strtoi(char **str, int *dest)
{
	return strtoi_base(str, dest, 0);
}
