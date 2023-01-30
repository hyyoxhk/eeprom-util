// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#ifndef _PARSING_LIBJSON_
#define _PARSING_LIBJSON_

struct layout;

int parse_json(struct layout *layout, const char *filename);

void get_field_string_with_size(void *e, const char *path, char *d, size_t n);

#define GET_FIELD_STRING(e, name, d) \
	get_field_string_with_size(e, name, d, sizeof(d))

#endif
