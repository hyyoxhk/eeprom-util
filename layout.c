#include "layout.h"

/*
 * new_layout() - Allocate a new layout based on the data given in buf.
 * @buf:	Data seed for layout
 * @buf_size:	Size of buf
 *
 * Allocates a new layout based on data in buf. The layout version is
 * automatically detected. The resulting layout struct contains a copy of the
 * provided data.
 *
 * Returns: pointer to a new layout on success, NULL on failure
 */
struct layout *new_layout(unsigned char *buf, unsigned int buf_size,
			  int layout_version,
			  enum print_format print_format)
{
	ASSERT(buf);

	struct layout *layout = malloc(sizeof(struct layout));
	if (!layout)
		return NULL;

	layout->layout_version = layout_version;
	layout->data = buf;
	layout->data_size = buf_size;

	if (build_layout(layout) < 0)
		return NULL;

	for (int i = 0; i < layout->num_of_fields; i++) {
		struct field *field = &layout->fields[i];
		init_field(field, buf, print_format);
		buf += field->ops->get_data_size(field);
	}

	layout->print = print_layout;
	layout->update_fields = update_fields;
	layout->update_bytes = update_bytes;
	layout->clear_fields = clear_fields;
	layout->clear_bytes = clear_bytes;

	return layout;
}

/*
 * free_layout() - a destructor for layout
 * @layout:	the layout to deallocate
 */
void free_layout(struct layout *layout)
{
	free(layout->fields);
	free(layout);
}
