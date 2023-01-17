// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#ifndef _UTIL_
#define _UTIL_

// Macro for printing error messages
#define eprintf(args...) fprintf (stderr, args)
// Macro for printing input error messages
#define ie_fmt(fmt) "Input error: " fmt " - Operation Aborted!\n"
#define ieprintf(fmt, ...) eprintf(ie_fmt(fmt), ##__VA_ARGS__)

#define STRTOI_STR_CON 1
#define STRTOI_STR_END 2

int strtoi_base(char **str, int *dest, int base);

int strtoi(char **str, int *dest);

#endif
