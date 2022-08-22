#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct editor_config
{
	int screenrows;
	int screencols;
	struct termios orig_termios;
};

struct editor_config E;

/*** terminal ***/

void die(const char* s)
{
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
	perror(s);
	exit(EXIT_FAILURE);
}

void disable_raw_mode()
{
	// Disable raw mode
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
		die("Error on tcsetattr");
}

void enable_raw_mode()
{
	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("Error on tcgetattr");
	atexit(disable_raw_mode);

	struct termios raw = E.orig_termios;

	raw.c_iflag &= ~(IXON | IXOFF | ICRNL | BRKINT | ISTRIP | INPCK);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	// Setting raw mode
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("Error on tcsetattr");
}

char editor_read_key()
{
	char c;
	int nchar;
	while ((nchar=read(STDIN_FILENO, &c, 1)) != 1)
	{
		if (nchar == -1 && errno != EAGAIN) die("Error on read");
	}
	return c;
}

int get_cursor_position(int* rows, int* cols)
{
	char buff[32];
	unsigned int i = 0;

	if (write(STDOUT_FILENO, "\x1b[6n", 4) == -1) return -1;

	while (i < sizeof(buff)-1)
	{
		if (read(STDIN_FILENO, &buff[i], 1) != 1) break;
		if (buff[i] == 'R') break;
		++i;
	}
	buff[i] = '\0';

	// parse response
	if (buff[0] != '\x1b' || buff[1] != '[') return -1;
	if (sscanf(&buff[2], "%d;%d", rows, cols) != 2) return -1;
	
	return 0;
}

int get_window_size(int* rows, int* cols)
{
	struct winsize ws;

	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
	{
		if (write(STDIN_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;	// Set cursor to the bottom-right corner
		return get_cursor_position(rows, cols);			// Get cursor position
	}
	else
	{
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		printf("%d %d\n", *rows, *cols);
		return 0;
	}
}

/*** output ***/

void editor_draw_rows()
{
	for (int i = 0; i < E.screenrows; ++i)
	{
		write(STDOUT_FILENO, "~", 1);
		if (i < E.screenrows-1)
			write(STDOUT_FILENO, "\r\n", 2);
	}
}

void editor_refresh_screen()
{
	write(STDOUT_FILENO, "\x1b[2J", 4);		// clears screen
	write(STDOUT_FILENO, "\x1b[H", 3);		// position the cursor at top-left corner
	editor_draw_rows();
	write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

void editor_process_key_pressed()
{
	char c = editor_read_key();

	switch (c)
	{
	case CTRL_KEY('q'):
		write(STDOUT_FILENO, "\x1b[2J", 4);
		write(STDOUT_FILENO, "\x1b[H", 3);
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}
}

/*** init ***/

void init_editor()
{
	if (get_window_size(&E.screenrows, &E.screencols) == -1)
		die("Error on get_window_size");
}

int main(void)
{
	enable_raw_mode();
	init_editor();
	while (1)
	{
		editor_refresh_screen();
		editor_process_key_pressed();
	}

	return EXIT_SUCCESS;
}
