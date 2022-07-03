#include <stdio.h>
#include <unistd.h>
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

	return 0;
}
