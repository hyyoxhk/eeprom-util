// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2022 He Yong <hyyoxhk@163.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <json-c/json.h>

#include "layout.h"
#include "field.h"
#include "parsing_libjson.h"

/*
 * This is to limit the structure (array) used to save the whole
 * path to the entry to be read.
 */
#define MAX_PARSED_NODES 20

static json_object *find_root(json_object *root, const char **names)
{
	json_object *node = root;

	while (*names) {
		const char *n = *names;
		json_object *cnode = NULL;

		if (json_object_object_get_ex(node, n, &cnode))
			node = cnode;
		else
			return NULL;

		names++;
	}

	return node;
}

static void get_value(json_object *e, void *dest)
{
	enum json_type type = json_type_null;

	type = json_object_get_type(e);
	if (type == json_type_int)
		*(unsigned int *)dest = json_object_get_int(e);
}

static void get_field(json_object *e, const char *path, void *dest)
{
	json_object *fld = NULL;

	if (path) {
		if (json_object_object_get_ex(e, path, &fld))
			get_value(fld, dest);
	} else {
		get_value(e, dest);
	}
}

static const char *get_field_string(json_object *e, const char *path)
{
	const char *str;
	json_object *node;

	if (path) {
		if (!json_object_object_get_ex(e, path, &node))
			return NULL;
	} else
		node = e;

	if (json_object_get_type(node) == json_type_string) {
		str = json_object_get_string(node);

		return str;
	}

	return NULL;
}

void get_field_string_with_size(void *e, const char *path, char *d, size_t n)
{
	const char *s = NULL;
	s = get_field_string(e, path);
	if (s)
		strncpy(d, s, n);
}

/*
 * Count number of elements in an array of strings
 * Last item must have a NULL terminator
 */
static unsigned int count_string_array(const char **nodes)
{
	const char **iter = nodes;
	int count = 0;

	while (*iter != NULL) {
		iter++;
		count++;
	}
	return count;
}

static bool path_append(const char **nodes, const char *field)
{
	unsigned int count = 0;

	count = count_string_array(nodes);

	if (count >= MAX_PARSED_NODES)
		return false;

	nodes[count++] = field;
	nodes[count] = NULL;

	return true;
}

static void *find_node_and_path(void *root, const char *field, const char **nodes)
{
	void *node = NULL;
	int i;

	if (!field)
		return NULL;

	for (i = 0; i < 2; i++) {
		nodes[0] = NULL;
		switch (i)
		{
		case 0:
			nodes[0] = "layout";
			nodes[1] = "field";
			nodes[2] = NULL;
			break;
		case 1:
			nodes[0] = "layout";
			nodes[1] = NULL;
			break;
		}

		if (find_root(root, nodes)) {
			if (!path_append(nodes, field))
				return NULL;

			node = find_root(root, nodes);
			if (node) {
				return node;
			}
		}
	}

	return NULL;
}

static void *find_node(void *root, const char *field)
{
	const char **nodes;
	void *node = NULL;

	if (!field)
		return NULL;

	nodes = (const char **)calloc(MAX_PARSED_NODES, sizeof(*nodes));
	if (!nodes)
		return NULL;

	node = find_node_and_path(root, field, nodes);
	free(nodes);

	return node;
}

static const char * const types[] = {
	"FIELD_BINARY",
	"FIELD_REVERSED",
	"FIELD_VERSION",
	"FIELD_ASCII",
	"FIELD_MAC",
	"FIELD_DATE",
	"FIELD_RESERVED",
	"FIELD_RAW"
};

static field_type get_index_by_name(const char *name)
{
	int i;

	if (!name)
		return FIELD_BINARY;

	for (i = 0; i < sizeof(types); i++)
		if (strcmp(types[i], name) == 0)
			return i;

	/* not found, default type 'FIELD_BINARY' */
	return FIELD_BINARY;
}

static void parse_field(void *setting, struct field *fields)
{
	void *elem;
	int count, i;
	const char *type = NULL;

	count = json_object_array_length(setting);
	for(i = 0; i < count; i++) {
		elem = json_object_array_get_idx(setting, i);

		if (!elem)
			continue;

		GET_FIELD_STRING(elem, "name", fields[i].name);
		GET_FIELD_STRING(elem, "short_name", fields[i].short_name);
		get_field(elem, "data_size", &fields[i].data_size);
		type = get_field_string(elem, "type");
		fields[i].type = get_index_by_name(type);
	}
}

static int parser(void *cfg, struct layout *layout)
{
	void *setting;
	int version = 0;

	if((setting = find_node(cfg, "version")) == NULL) {
		printf("Missing version in configuration file.\n");
		return -1;
	}
	get_field(setting, NULL, &version);
	if (version != layout->version) {
		printf("opened with wrong version file.\n");
		return -1;
	}

	if((setting = find_node(cfg, "field")) == NULL) {
		printf("Missing field in configuration file");
		return -1;
	}

	layout->num_of_fields = json_object_array_length(setting);
	layout->fields = calloc(layout->num_of_fields, sizeof(struct field));

	parse_field(setting, layout->fields);

	return 0;
}

int parse_json(struct layout *layout, const char *filename)
{
	int fd, ret;
	struct stat stbuf;
	unsigned int size;
	char *string;
	json_object *cfg;

	/* Read the file. If there is an error, report it and exit. */
	ret = stat(filename, &stbuf);
	if (ret)
		return -EBADF;

	size = stbuf.st_size;
	string = (char *)malloc(size + 1);
	if (!string)
		return -ENOMEM;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		free(string);
		return -EBADF;
	}

	ret = read(fd, string, size);
	close(fd);
	if (ret < 0) {
		ret = -errno;
		free(string);
		return ret;
	}

	if (ret != size) {
		printf("partial read of %s, proceeding anyway", filename);
	}
	string[ret] = '\0';

	cfg = json_tokener_parse(string);
	if (!cfg) {
		printf("JSON File corrupted");
		free(string);
		return -1;
	}

	ret = parser(cfg, layout);

	json_object_put(cfg);

	free(string);

	return ret;
}
