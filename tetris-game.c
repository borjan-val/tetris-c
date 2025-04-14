#include "tetris-game.h"
#include <stdlib.h>
#include <time.h>

inline bool cell_out_of_bounds(struct tetris_game_piece pc, unsigned char x,
			       unsigned char y)
{
	return PIECE_DATA[pc.idx][pc.idx][y][x] != 0 &&
	       (pc.x + x >= 10 || pc.x + x < 0 || pc.y + y >= 20 ||
		pc.y + y < 0);
}

inline bool cell_overlaps(struct tetris_game_piece pc,
			  unsigned char board[20][10], unsigned char x,
			  unsigned char y)
{
	return PIECE_DATA[pc.idx][pc.rot][y][x] != 0 &&
	       board[pc.y + y][pc.x + x] != 0;
}

inline void set_cell(struct tetris_game_piece pc, unsigned char board[20][10],
		     unsigned char x, unsigned char y)
{
	unsigned char cell = PIECE_DATA[pc.idx][pc.rot][y][x];
	if (cell == 0)
		return;
	board[pc.y + y][pc.x + x] = cell;
	return;
}

bool collides(struct tetris_game_piece pc, unsigned char board[20][10])
{
	for (unsigned char y = 0; y < 4; y++) {
		for (unsigned char x = 0; x < 4; x++) {
			if (cell_out_of_bounds(pc, x, y))
				return 1;
			if (cell_overlaps(pc, board, x, y))
				return 1;
		}
	}
	return 0;
}

bool line_full(unsigned char board[20][10], unsigned char y)
{
	for (unsigned char x = 0; x < 10; x++) {
		if (board[y][x] == 0)
			return 0;
	}
	return 1;
}

void place_piece_down(struct tetris_game *tgptr)
{
	for (unsigned char y = 0; y < 4; y++) {
		for (unsigned char x = 0; x < 4; x++) {
			if (cell_out_of_bounds(tgptr->pc, x, y))
				continue;
			set_cell(tgptr->pc, tgptr->board, x, y);
		}
	}
}

void new_piece(struct tetris_game *tgptr)
{
	struct tetris_game_piece newpc;
	newpc.idx = tgptr->npcidx;
	newpc.rot = 0;
	newpc.x = 3;
	newpc.y = 0;
	tgptr->pc = newpc;
	tgptr->npcidx = rand() % 7;
}

struct tetris_game *new_tetris_game()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	srand(ts.tv_sec + ts.tv_nsec);
	struct tetris_game tg;
	tg.npcidx = rand() % 7;
	new_piece(&tg);
	for (unsigned char y = 0; y < 20; y++) {
		for (unsigned char x = 0; x < 10; x++) {
			tg.board[y][x] = 0;
		}
	}
	tg.lines = 0;
	tg.score = 0;
	return &tg;
}

struct tetris_game_result tetris_game_update(struct tetris_game *tgptr)
{
	struct tetris_game_result result;
	result.game_ended = 0;
	result.lines_cleared[0] = 0xFF;
	struct tetris_game_piece nextpc = tgptr->pc;
	nextpc.y++;
	if (!collides(nextpc, tgptr->board)) {
		tgptr->pc = nextpc;
		return result;
	}
	place_piece_down(tgptr);
	unsigned char size = 0;
	for (unsigned char y = tgptr->pc.y; y < tgptr->pc.y + 4; y++) {
		if (line_full(tgptr->board, y)) {
			result.lines_cleared[size] = y;
			if (++size < 4)
				result.lines_cleared[size] = 0xFF;
		}
	}
	tgptr->score += CLEAR_REWARDS[size] * ((tgptr->lines / 10) + 1);
	tgptr->lines += size;
	new_piece(tgptr);
	result.game_ended = collides(tgptr->pc, tgptr->board);
	return result;
}