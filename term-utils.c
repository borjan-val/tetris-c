#include "term-utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define ALT_BUF_ENABLE "\033[?1049h"
#define ALT_BUF_DISABLE "\033[?1049l"
#define RESET_BUF "\033[2J\033[1;1H"

static struct termios orig_stdin;
static unsigned char t_reg = 0;
static unsigned char t_frozen = 0;

// Switch to the alternative terminal buffer in order to not interfere with
// the users terminal buffer going into the program
static inline void enable_alt_buf()
{
	write(STDOUT_FILENO, ALT_BUF_ENABLE, 8);
}

// Disable the alternative buffer to return the user to their own terminal
// buffer going out of the program
static inline void disable_alt_buf()
{
	write(STDOUT_FILENO, ALT_BUF_DISABLE, 8);
}

// Given a color and boolean to determine whether to set fore- or background
// color, construct a fixed-size string of the SGR parameter ANSI escape
// sequence used to set that color
static struct str color_str_builder(enum term_color color, unsigned char bg)
{
	struct str str;
	str.arr = malloc(5);
	str.len = 5;
	str.arr[0] = '\033';
	str.arr[1] = '[';
	str.arr[2] = '3' + bg;
	str.arr[3] = '0' + color;
	str.arr[4] = 'm';
	return str;
}

static void t_setup_stdin()
{
	// Save old STDIN terminal attributes
	struct termios t_stdin;
	tcgetattr(STDIN_FILENO, &t_stdin);
	orig_stdin = t_stdin;

	// Disable canonical mode, echo and signals;
	// make noncanonical read immediate
	t_stdin.c_lflag &= ~(ECHO | ICANON | ISIG);
	t_stdin.c_cc[VMIN] = 0;
	t_stdin.c_cc[VTIME] = 0;

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &t_stdin);
}

// Return STDIN to its initial state
static inline void t_revert_stdin()
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_stdin);
}

// Undo all setup done by registering term-utils
static void t_cleanup()
{
	if (!t_reg)
		return;

	t_thaw();
	t_reset();
	t_revert_stdin();
	disable_alt_buf();
}

// A signal handler that properly exits with terminal cleanup.
static void graceful_exit(int sig)
{
	(void)sig;
	exit(1);
}

void t_register()
{
	if (t_reg)
		return;

	enable_alt_buf();
	t_setup_stdin();
	t_reset();

	// Make sure terminal cleanup happens no matter how the process dies.
	atexit(t_cleanup);
	signal(SIGTERM, graceful_exit);
	signal(SIGINT, graceful_exit);
	signal(SIGHUP, graceful_exit);

	t_reg = 1;
}

void t_print(struct str str)
{
	// Text to be printed is held in a buffer on the heap, which may be up
	// to 0xFFFF characters in length
	static struct str buf;
	static unsigned short size = 0;
	if (size == 0) {
		size = 0x10;
		buf.arr = malloc(size);
	}
	// Only copy the given string into the buffer if it is not empty
	if (str.len == 0)
		goto output;
	// Clamp the number of bytes to be copied to enforce the maximum buffer
	// length
	unsigned short n = (0xFFFF - str.len < buf.len) ? 0xFFFF - buf.len :
							  str.len;
	// If the number of bytes allocated to the buffer is not enough to
	// store all new data, double the buffer size until it fits, then
	// reallocate
	unsigned short new_size = size;
	while (buf.len + n > new_size) {
		new_size <<= 1;
		if (new_size == 0)
			new_size = 0xFFFF;
	}
	if (new_size > size)
		buf.arr = realloc(buf.arr, new_size);
	size = new_size;
	// Copy the data and update the string length
	memcpy(buf.arr + buf.len, str.arr, n);
	buf.len += n;

output:
	// If the terminal isn't frozen, write the buffer to STDOUT and flush it
	if (t_frozen)
		return;
	write(STDOUT_FILENO, buf.arr, buf.len);
	size = 0;
	buf.len = 0;
	free(buf.arr);
}

void t_printc(char c)
{
	t_print((struct str){ &c, 1 });
}

struct str t_read()
{
	struct str str;
	str.len = 0;
	unsigned short size = 0x10;
	char c;
	str.arr = malloc(size);
	// Read from STDIN one byte at a time, doubling the size of the array
	// used to hold the input string each time it is filled
	while (read(STDIN_FILENO, &c, 1)) {
		if (size == 0xFFFF)
			break;
		if (str.len >= size) {
			size <<= 1;
			if (size == 0)
				size = 0xFFFF;
			str.arr = realloc(str.arr, size);
		}
		str.arr[str.len++] = c;
	}
	// If there was no input, free the string array pointer, else resize
	// the array to the exact length of the string.
	if (str.len == 0)
		free(str.arr);
	else if (str.len < size)
		str.arr = realloc(str.arr, str.len);

	return str;
}

void t_fg_color(enum term_color color)
{
	struct str str = color_str_builder(color, 0);
	t_print(str);
	free(str.arr);
}

void t_bg_color(enum term_color color)
{
	struct str str = color_str_builder(color, 1);
	t_print(str);
	free(str.arr);
}

void t_goto(unsigned char col, unsigned char row) {
	struct str str;
	// Construct the ANSI escape sequence to move the cursor to the given
	// coordinates
	str.arr = malloc(11);
	str.arr[0] = '\033';
	str.arr[1] = '[';
	snprintf(str.arr + 2, 4, "%u", row + 1);
	unsigned char a = 4 + (row > 8) + (row > 98);
	str.arr[a] = ';';
	snprintf(str.arr + a + 1, 4, "%u", col + 1);
	unsigned char b = a + 2 + (col > 8) + (col > 98);
	str.arr[b] = 'H';
	if (b < 10)
		str.arr = realloc(str.arr, b + 1);
	str.len = b + 1;

	t_print(str);

	free(str.arr);
}

void t_freeze()
{
	t_frozen = 1;
}

void t_thaw()
{
	t_frozen = 0;
	t_print((struct str){ NULL, 0 });
}

void t_reset()
{
	t_print((struct str){ RESET_BUF, 10 });
}