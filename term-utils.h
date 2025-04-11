#ifndef TERM_UTILS_H
#define TERM_UTILS_H

#define ALT_BUF_ENABLE "\033[?1049h"
#define ALT_BUF_DISABLE "\033[?1049l"
#define CLEAR_BUF "\033[3J"
#define RESET_CUR_POS "\033[1;1H"
#define FG_BLACK "\033[30m"
#define FG_RED "\033[31m"
#define FG_GREEN "\033[32m"
#define FG_YELLOW "\033[33m"
#define FG_BLUE "\033[34m"
#define FG_MAGENTA "\033[35m"
#define FG_CYAN "\033[36m"
#define FG_WHITE "\033[37m"
#define BG_BLACK "\033[40m"
#define BG_RED "\033[41m"
#define BG_GREEN "\033[42m"
#define BG_YELLOW "\033[43m"
#define BG_BLUE "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN "\033[46m"
#define BG_WHITE "\033[47m"

// Pause output
static inline void stdout_freeze();

// Resume output
static inline void stdout_thaw();

// Switch terminal to raw mode to intercept input directly
void stdin_raw();

// Restore default terminal behaviour (usually cooked mode)
void stdin_revert();

#endif