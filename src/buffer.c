#include "buffer.h"

void init_buffer(struct buff_st* buffer, size_t size) {
	buffer->buff = (char*) malloc(size * sizeof(char));
	buffer->size = 0;
	buffer->cursor = 0;
}

void destroy_buffer(struct buff_st* buffer) {
	free(buffer->buff);
	buffer->buff = NULL;
	buffer->size = 0;
	buffer->cursor = 0;
}

void move_cursor_to_start(struct buff_st* buffer) {
	buffer->cursor = 0;
}

void move_cursor_to_end(struct buff_st* buffer) {
	buffer->cursor = buffer->size - 1;
}

void move_cursor(struct buff_st* buffer, int position) {
	return;
}
