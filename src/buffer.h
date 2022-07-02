#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <string.h>

#define INIT_SIZE_BUFF 255

struct buff_struct
{
	char* buff;
	int nb;
	int max_n;
};

void init_buffer(struct buff_struct* buffer, size_t size);
void free_buffer(struct buff_struct* buffer);

#endif	/* BUFFER_H */
