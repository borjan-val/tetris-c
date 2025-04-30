#define _POSIX_C_SOURCE 199506L

#include "tetris.h"
#include "tetris-game.h"
#include "term-utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NS_IN_MS 1000000

static const unsigned char GRAVITY_SPEEDS[29] = {
	48, 43, 38, 33, 28, 23, 18, 13, 8, 6, 5, 5, 5, 4, 4,
	4,  3,	3,  3,	2,  2,	2,  2,	2, 2, 2, 2, 2, 2
};

static const enum term_color COLORMAP[8] = { UNSET, RED,   RED,	   MAGENTA,
					     BLUE,  GREEN, YELLOW, CYAN };

static const enum tetris_game_input KEYMAP[256] = {
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   HARD_DROP,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	QUIT,	    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	MOVE_LEFT,
	INVALID, INVALID,   MOVE_RIGHT, ROTATE_CW,  INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	ROTATE_CCW, INVALID,	SOFT_DROP,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, MOVE_LEFT, INVALID,	INVALID,    MOVE_RIGHT, ROTATE_CW,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	ROTATE_CCW,
	INVALID, SOFT_DROP, INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID,    INVALID,	INVALID,
	INVALID, INVALID,   INVALID,	INVALID
};

static const unsigned char EMPTY_SHADOW[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static void copy_shadow(unsigned char (*dest)[10],
			const unsigned char (*src)[10])
{
	for (unsigned char x = 0; x < 10; x++) {
		(*dest)[x] = (*src)[x];
	}
}

static void get_shadow(unsigned char (*dest)[10], struct tetris_game_piece pc)
{
	copy_shadow(dest, &EMPTY_SHADOW);
	for (unsigned char x = 0; x < 10; x++) {
		if (x < pc.x || x > pc.x + 3)
			continue;
		for (unsigned char y = 0; y < 4; y++) {
			if (PIECE_DATA[pc.idx][pc.rot][y][x - pc.x] != 0) {
				(*dest)[x] = 1;
				break;
			}
		}
	}
}

static void render_state(enum term_color (*board)[20][10],
			 enum term_color (*npcmat)[4][4],
			 const unsigned char (*pshadow)[10],
			 unsigned short lines, unsigned int score)
{
	t_freeze();
	t_reset();
	t_print((struct str){ "┏━━━━━━━━━━━━━━━━━━━━┯━━━━━━━━┓\n", 94 });
	for (unsigned char y = 0; y < 20; y++) {
		t_print((struct str){ "┃", 3 });

		for (unsigned char x = 0; x < 10; x++) {
			t_bg_color((*board)[y][x]);
			t_print((struct str){ "  ", 2 });
		}
		t_bg_color(UNSET);

		if (y == 5) {
			t_print((struct str){ "├────────┨\n", 31 });
			continue;
		}

		t_print((struct str){ "│", 3 });

		if (y < 4) {
			for (unsigned char x = 0; x < 4; x++) {
				t_bg_color((*npcmat)[y][x]);
				t_print((struct str){ "  ", 2 });
			}
			t_bg_color(UNSET);
			t_print((struct str){ "┃\n", 4 });
			continue;
		}

		char linebuf[9];
		switch (y) {
		case 4:
			t_print((struct str){ "  NEXT  ", 8 });
			break;
		case 12:
			t_print((struct str){ "Level:  ", 8 });
			break;
		case 13:
			snprintf(linebuf, 9, "%-8u", lines / 10);
			t_print((struct str){ linebuf, 8 });
			break;
		case 15:
			t_print((struct str){ "Lines:  ", 8 });
			break;
		case 16:
			snprintf(linebuf, 9, "%-8u", lines);
			t_print((struct str){ linebuf, 8 });
			break;
		case 18:
			t_print((struct str){ "Score:  ", 8 });
			break;
		case 19:
			snprintf(linebuf, 9, "%08X", score);
			t_print((struct str){ linebuf, 8 });
			break;
		default:
			t_print((struct str){ "        ", 8 });
			break;
		}

		t_print((struct str){ "┃\n", 4 });
	}
	t_print((struct str){ "┗", 3 });
	for (unsigned char x = 0; x < 10; x++) {
		if ((*pshadow)[x])
			t_print((struct str){ "╍╍", 6 });
		else
			t_print((struct str){ "━━", 6 });
	}
	t_print((struct str){ "┷━━━━━━━━┛\n", 31 });

	t_thaw();
}

static void map_board(enum term_color (*dest)[20][10],
		      unsigned char (*src)[20][10])
{
	for (unsigned char y = 0; y < 20; y++) {
		for (unsigned char x = 0; x < 10; x++) {
			(*dest)[y][x] = COLORMAP[(*src)[y][x]];
		}
	}
}

static void map_piece(enum term_color (*dest)[4][4],
		      struct tetris_game_piece src)
{
	for (unsigned char y = 0; y < 4; y++) {
		for (unsigned char x = 0; x < 4; x++) {
			(*dest)[y][x] =
				COLORMAP[PIECE_DATA[src.idx][src.rot][y][x]];
		}
	}
}

static void mboard_push_up(enum term_color (*mboard)[20][10], unsigned char l)
{
	for (unsigned char y = 0; y < l; y++) {
		for (unsigned char x = 0; x < 10; x++) {
			(*mboard)[y][x] = (*mboard)[y + 1][x];
		}
	}
	for (unsigned char x = 0; x < 10; x++) {
		(*mboard)[l][x] = WHITE;
	}
}

static void mboard_insert_clear_lines(enum term_color (*mboard)[20][10],
				      unsigned char (*lines_cleared)[4])
{
	unsigned char lctr = 0;

	for (; (*lines_cleared)[lctr] != 0xFF && lctr < 4; lctr++) {
	}

	for (unsigned char i = 0; i < lctr; i++) {
		mboard_push_up(mboard, (*lines_cleared)[lctr - 1 - i]);
	}
}

static void mssleep(unsigned int ms)
{
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = ms * NS_IN_MS;
	nanosleep(&ts, &ts);
}

static void render_result(struct tetris_game *tgptr,
			  struct tetris_game_result result)
{
	unsigned char rboard[20][10];
	copy_board(&rboard, &tgptr->board);

	enum term_color npcmat[4][4];
	map_piece(&npcmat,
		  (struct tetris_game_piece){ 0, 0, tgptr->npcidx, 0 });

	if (result.lines_cleared[0] == 0xFF)
		goto no_clear_anim;

	enum term_color tboard[20][10];

	map_board(&tboard, &rboard);
	mboard_insert_clear_lines(&tboard, &result.lines_cleared);

	render_state(&tboard, &npcmat, &EMPTY_SHADOW, tgptr->lines,
		     tgptr->score);

	unsigned char lctr = 0;

	for (; result.lines_cleared[lctr] != 0xFF && lctr < 4; lctr++) {
	}

	for (unsigned char i = 0; i < 5; i++) {
		for (unsigned char j = 0; j < lctr; j++) {
			tboard[result.lines_cleared[j]][4 - i] = UNSET;
			tboard[result.lines_cleared[j]][5 + i] = UNSET;
		}
		mssleep(50);
		render_state(&tboard, &npcmat, &EMPTY_SHADOW, tgptr->lines,
			     tgptr->score);
	}
	mssleep(50);

no_clear_anim:
	place_piece_down(&rboard, tgptr->pc);

	enum term_color mboard[20][10];
	map_board(&mboard, &rboard);

	unsigned char shadow[10];
	get_shadow(&shadow, tgptr->pc);

	render_state(&mboard, &npcmat, &shadow, tgptr->lines, tgptr->score);
}

static void handle_input(struct tetris_game *tgptr, char c)
{
	enum tetris_game_input i = KEYMAP[(unsigned char)c];

	struct tetris_game_result r;
	switch (i) {
	case QUIT:
		exit(0);
		break;
	case INVALID:
		return;
	default:
		r = tetris_game_input(tgptr, i);
		render_result(tgptr, r);
		if (r.game_ended)
			exit(0);
		break;
	}
}

int main()
{
	t_register();

	struct tetris_game *tgptr = new_tetris_game();
	render_result(tgptr, (struct tetris_game_result){ 0, 0, { 0xFF } });

	while (1) {
		unsigned char f = (tgptr->lines / 10 < 29) ?
					  GRAVITY_SPEEDS[tgptr->lines / 10] :
					  1;
		while (f-- != 0) {
			mssleep(17);
			struct str str = t_read();
			for (unsigned short i = 0; i < str.len; i++) {
				handle_input(tgptr, str.arr[i]);
			}
			if (str.len > 0)
				free(str.arr);
		}
		struct tetris_game_result result = tetris_game_update(tgptr);
		if (result.game_ended)
			exit(0);
		render_result(tgptr, result);
	}
}