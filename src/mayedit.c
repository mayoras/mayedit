#define MAYEDIT_VERSION "0.0.1"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

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
		return get_cursor_position(rows, cols);					// Get cursor position
	}
	else
	{
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		printf("%d %d\n", *rows, *cols);
		return 0;
	}
}

/*** append buffer ***/

struct abuff
{
    char* b;
    int len;
};

#define APBUFF_INIT {NULL, 0}

void ab_append(struct abuff* ab, const char* s, int len)
{
    char* new = realloc(ab->b, ab->len + len);

    if (new == NULL) return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void ab_free(struct abuff* ab)
{
    free(ab->b);
}

/*** output ***/

void editor_draw_rows(struct abuff* ab)
{
	for (int i = 0; i < E.screenrows; ++i)
	{
	    if (i == E.screenrows / 3)
	    {
		char welcome[80];
		int welcome_len = snprintf(welcome, sizeof(welcome),
			"MAYORA text editor -- version %s", MAYEDIT_VERSION);
		if (welcome_len > E.screencols) welcome_len = E.screencols;
		int padding = (E.screencols - welcome_len) / 2;
		if (padding)
		{
		    ab_append(ab, "~", 1);
		    padding--;
		}
		while (padding--) ab_append(ab, " ", 1);
		ab_append(ab, welcome, welcome_len);
	    }
	    else
	    {
		ab_append(ab, "~", 1);
	    }
	    ab_append(ab, "\x1b[K", 3);    // clears part of the line (right)
	    if (i < E.screenrows-1)
		ab_append(ab, "\r\n", 2);
	}
}

void editor_refresh_screen()
{
    struct abuff ab = APBUFF_INIT;

    ab_append(&ab, "\x1b[?25l", 6);	// Hides the cursor on refreshing
    //ab_append(&ab, "\x1b[2J", 4);	// clears screen (Don't use)
    ab_append(&ab, "\x1b[H", 3);        // position the cursor at top-left corner

    editor_draw_rows(&ab);

    ab_append(&ab, "\x1b[H", 3);
    ab_append(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    ab_free(&ab);
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
