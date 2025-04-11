#include "term-utils.h"
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_stdin;
bool stdin_modified = 0;

static inline void stdout_freeze()
{
	tcflow(STDOUT_FILENO, TCOOFF);
}

static inline void stdout_thaw()
{
	tcflow(STDOUT_FILENO, TCOON);
}

void stdin_raw()
{
	tcgetattr(STDIN_FILENO, &orig_stdin);
	struct termios raw_stdin = orig_stdin;
	raw_stdin.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSANOW, &raw_stdin);
	stdin_modified = 1;
	atexit(stdin_revert);
}

void stdin_revert()
{
	if (stdin_modified)
		tcsetattr(STDIN_FILENO, TCSANOW, &orig_stdin);
}