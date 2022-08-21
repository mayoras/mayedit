#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#define CTRL_KEY(k) ((k) & 0x1f)

struct termios orig_term;

void die(const char* s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

void disable_raw_mode()
{
	// Disable raw mode
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term) == -1)
		die("Error on tcsetattr");
}

void enable_raw_mode()
{
	if (tcgetattr(STDIN_FILENO, &orig_term) == -1) die("Error on tcgetattr");
	atexit(disable_raw_mode);

	struct termios raw = orig_term;

	raw.c_iflag &= ~(IXON | IXOFF | ICRNL | BRKINT | ISTRIP | INPCK);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	// Setting raw mode
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("Error on tcsetattr");
}

int main(void)
{
	enable_raw_mode();

	while (1) {
		char c = '\0';
		if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("Error on read");
		if (!isprint(c)) {
			printf("%d\r\n", c);
		} else {
			printf("%d (%c)\r\n", (int)c, c);
		}
		if (c == CTRL_KEY('q')) break;
	}

	return EXIT_SUCCESS;
}