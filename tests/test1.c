#include <criterion/criterion.h>
#include "../src/buffer.h"
#include <stdio.h>

Test(buffer, init_buffer_test) {
	struct buff_st buffer;
	buffer.buff = NULL;				// Avoid pointer to garbage (C WARNING)
	char* ptr = buffer.buff;	// Ptr to tell if memory was allocated afterwards

	init_buffer(&buffer, INIT_SIZE_BUFF);

	cr_assert(buffer.buff != ptr);
	cr_assert(buffer.size==0);
	cr_assert(buffer.cursor==0);

	destroy_buffer(&buffer);
	free(ptr);
}

Test(buffer, destroy_buffer_test) {
	struct buff_st buffer;
	init_buffer(&buffer, INIT_SIZE_BUFF);

	destroy_buffer(&buffer);
	cr_assert(buffer.buff == NULL);
	cr_assert(buffer.size==0);
	cr_assert(buffer.cursor==0);
}