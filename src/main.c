#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <ncurses.h>

#include "buffer.h"

void write_ch(char* buffer, int nb, char c) {
	*buffer = c;
	++buffer;
}

void write_line(struct buff_struct* buffer, char* line, int nchars) {
	for (int i = 0; i < nchars && buffer->nb < buffer->max_n; ++i) {
		buffer->buff[buffer->nb++] = line[i];
	}

	if (buffer->nb == buffer->max_n) {
		printf("Buffer is full. Need to reallocate\n");
	}
}

int main(void) {

	struct buff_struct buffer;
	init_buffer(&buffer, INIT_SIZE_BUFF);

	// If buffer is not allocated
	if (buffer.buff == NULL) {
		perror("Error: Could not allocate buffer\n");
		exit(EXIT_FAILURE);
	}

	// scanf("%s", buffer);

	char* line;
	size_t nl = 32;
	int n_chars = getline(&line, &nl, stdin);

	write_line(&buffer, line, n_chars);

	printf("%s\n", buffer.buff);

	free_buffer(&buffer);

	return 0;
}