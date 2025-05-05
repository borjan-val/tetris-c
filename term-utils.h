#ifndef TERM_UTILS_H
#define TERM_UTILS_H

// Store a fixed-size string
// WARNING: DO NOT EXPECT NULL TERMINATION!
struct str {
	char *arr;
	unsigned short len;
};

// Possible terminal colors
enum term_color {
	BLACK = 0,
	RED = 1,
	GREEN = 2,
	YELLOW = 3,
	BLUE = 4,
	MAGENTA = 5,
	CYAN = 6,
	WHITE = 7,
	UNSET = 9
};

// Print to the term-utils window
void t_print(struct str str);

// Print single character to the term-utils window
void t_printc(char c);

// Read a string from the term-utils window
struct str t_read();

// Set text color of term-utils window
void t_fg_color(enum term_color color);

// Set text background color of term-utils window
void t_bg_color(enum term_color color);

// Go to a given position of the term-utils window
void t_goto(unsigned char col, unsigned char row);

// Open term-utils window and configure automatic cleanup on close
void t_register();

// Freeze term-utils window output
void t_freeze();

// Unfreeze ("thaw") term-utils window output
void t_thaw();

// Clear term-utils window
void t_reset();

#endif