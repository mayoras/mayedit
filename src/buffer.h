#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <string.h>

#define INIT_SIZE_BUFF 255

struct buff_st
{
	char* buff;			// String buffer
	int size;				// Number of characters
	// int max_n;			// Max capacity (memory allocated)
	int cursor;			// Write cursor
};

void init_buffer(struct buff_st* buffer, size_t size);
void destroy_buffer(struct buff_st* buffer);

void move_cursor_to_start(struct buff_st* buffer);
void move_cursor_to_end(struct buff_st* buffer);

#endif	/* BUFFER_H */
