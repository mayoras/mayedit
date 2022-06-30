#include "buffer.h"

void init_buffer(struct buff_struct* buffer, size_t size) {
	buffer->buff = (char*) malloc(size * sizeof(char));
	buffer->nb = 0;
	buffer->max_n = size;
}

void free_buffer(struct buff_struct* buffer) {
	free(buffer->buff);
}
