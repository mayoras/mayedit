#define MAYEDIT_VERSION "0.0.1"

#define _DEFAULT_SOURCE
#define _GNU_SOURCE
#define _BSD_SOURCE

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define ESC_SEQ_SZ 3

/*** data ***/

enum ED_KEYS {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
    DEL_KEY
};

typedef struct {
    int size;
    char* chars;
} erow;

struct editor_config
{
    int cx, cy;	    // Cursor position

    // row and col offset from screen
    int rowoff;
    int coloff;

    int screenrows;
    int screencols;
    int numrows;
    erow* row;
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

int editor_read_key()
{
    char c;
    int nchar;
    while ((nchar=read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nchar == -1 && errno != EAGAIN) die("Error on read");
    }

    if (c == '\x1b')
    {
	char seq[ESC_SEQ_SZ];

	if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
	if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

	if (seq[0] == '[')
	{
	    // <esc>[(0-9)~
	    if (seq[1] <= 9 && seq[1] >= 0)
	    {
		if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
		if (seq[2] == '~')
		    switch(seq[1])
		    {
			case '1': return HOME_KEY;
			case '3': return DEL_KEY;	// TODO: Implement delete key functionality
			case '4': return END_KEY;
		        case '5': return PAGE_UP;
		        case '6': return PAGE_DOWN;
			case '7': return HOME_KEY;
			case '8': return END_KEY;
		        default:break;
		    }
	    } else	// <esc>[(A|B|C|D)
		switch(seq[1])
	    	{
	    	    case 'A': return ARROW_UP;
	    	    case 'B': return ARROW_DOWN;
	    	    case 'C': return ARROW_RIGHT;
	    	    case 'D': return ARROW_LEFT;
		    case 'H': return HOME_KEY;
		    case 'F': return END_KEY;
	    	    default:break;
	    	}
	} else
	    if (seq[0] == 'O')
		switch (seq[1])
		{
		    case 'H': return HOME_KEY;
		    case 'F': return END_KEY;
		    default: break;
		}
	return '\x1b';
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
	} else
	{
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		printf("%d %d\n", *rows, *cols);
		return 0;
	}
}

/*** row operations ***/

void editor_append_row(char* s, size_t len)
{
    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));

    int at = E.numrows;
    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';
    E.numrows++;
}

/*** file i/o ***/

void editor_open(char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (!fp) die("Error on fopen");
    
    char* line = NULL;
    size_t line_cap = 0;
    ssize_t line_len;
    while ((line_len=getline(&line, &line_cap, fp)) != -1) {	// Allocates in line
        // Strip new line or carriage chars from the line before copying it
        while (line_len > 0 && (line[line_len-1] == '\n' || line[line_len-1] == '\r'))
            line_len--;
        editor_append_row(line, line_len);
    }
    free(line);
    fclose(fp);
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

void editor_scroll()
{
    if (E.cx < E.coloff) {
	E.coloff = E.cx;
    }
    if (E.cy < E.rowoff) {
	E.rowoff = E.cy;
    }

    if (E.cx >= E.screencols + E.coloff) {
	E.coloff = E.cx - E.screencols + 1;
    }
    if (E.cy >= E.screenrows + E.rowoff) {
	E.rowoff = E.cy - E.screenrows + 1;
    }
}

void editor_draw_rows(struct abuff* ab)
{
	for (int i = 0; i < E.screenrows; ++i)
	{
	    int filerow = i + E.rowoff;
	    if (filerow >= E.numrows) {
		if (E.numrows == 0 && i == E.screenrows / 3)
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
	    	} else
	    	{
	    	    ab_append(ab, "~", 1);
	    	}
	    } else {
		int len = E.row[filerow].size - E.coloff;
		if (len < 0) len = 0;
		if (len > E.screencols) len = E.screencols;
		ab_append(ab, &E.row[filerow].chars[E.coloff], len);
	    }
	    ab_append(ab, "\x1b[K", 3);    // clears part of the line (right)
	    if (i < E.screenrows-1)
		ab_append(ab, "\r\n", 2);
	}
}

void editor_refresh_screen()
{
    editor_scroll();
    struct abuff ab = APBUFF_INIT;

    ab_append(&ab, "\x1b[?25l", 6);	// Hides the cursor on refreshing
    //ab_append(&ab, "\x1b[2J", 4);	// clears screen (Don't use)
    ab_append(&ab, "\x1b[H", 3);        // position the cursor at top-left corner

    editor_draw_rows(&ab);

    // Position the cursor
    char buff[32];
    snprintf(buff, sizeof(buff), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, (E.cx - E.coloff) + 1);
    ab_append(&ab, buff, strlen(buff));

    ab_append(&ab, "\x1b[?25h", 6);
    write(STDOUT_FILENO, ab.b, ab.len);
    ab_free(&ab);
}

/*** input ***/

void editor_move_cursor(int key) {
    switch(key)
    {
	// Arrow keys
	case ARROW_UP:
	    if (E.cy > 0)
		E.cy--;
	    break;
	case ARROW_LEFT:
	    if (E.cx > 0)
		E.cx--;
	    break;
	case ARROW_DOWN:
	    if (E.cy < E.numrows)
		E.cy++;
	    break;
	case ARROW_RIGHT:
	    E.cx++;
	    break;
	case HOME_KEY:
	    E.cx = 0;
	    break;
	case END_KEY:
	    E.cx = E.screencols - 1;
	    break;
	default:break;
    }
}

void editor_process_key_pressed()
{
	int c = editor_read_key();

	switch (c)
	{
	    case CTRL_KEY('q'):
		write(STDOUT_FILENO, "\x1b[2J", 4);
		write(STDOUT_FILENO, "\x1b[H", 3);
		exit(EXIT_SUCCESS);
		break;
	    case ARROW_UP:
	    case ARROW_LEFT:
	    case ARROW_DOWN:
	    case ARROW_RIGHT: 
	    case HOME_KEY:
	    case END_KEY:
		editor_move_cursor(c); break;

	    case PAGE_UP:
	    case PAGE_DOWN:
		{
		    int times = E.screenrows;
		    while (times--)
			editor_move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
		}
		break;
	    default:
		break;
	}
}

/*** init ***/

void init_editor()
{
    E.cx = 0;
    E.cy = 0;
    E.numrows = 0;
    E.row = NULL;
    E.rowoff = 0;
    E.coloff= 0;
    if (get_window_size(&E.screenrows, &E.screencols) == -1) die("Error on get_window_size");
}

int main(int argc, char** argv)
{
    enable_raw_mode();
    init_editor();
    if (argc >= 2) {
	char* filename = argv[1];
	editor_open(filename);
    }
    while (1)
    {
        editor_refresh_screen();
        editor_process_key_pressed();
    }
    
    return 0;
}
