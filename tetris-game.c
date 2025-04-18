#include "tetris-game.h"
#include <stdlib.h>
#include <time.h>

static inline void seed_random()
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	srand(ts.tv_sec + ts.tv_nsec);
}

static inline bool cell_out_of_bounds(struct tetris_game_piece pc,
				      unsigned char x, unsigned char y)
{
	return PIECE_DATA[pc.idx][pc.idx][y][x] != 0 &&
	       (pc.x + x >= 10 || pc.x + x < 0 || pc.y + y >= 20 ||
		pc.y + y < 0);
}

static inline bool cell_overlaps(struct tetris_game_piece pc,
				 unsigned char board[20][10], unsigned char x,
				 unsigned char y)
{
	return PIECE_DATA[pc.idx][pc.rot][y][x] != 0 &&
	       board[pc.y + y][pc.x + x] != 0;
}

static inline void set_cell(struct tetris_game_piece pc,
			    unsigned char board[20][10], unsigned char x,
			    unsigned char y)
{
	unsigned char cell = PIECE_DATA[pc.idx][pc.rot][y][x];
	if (cell == 0)
		return;
	board[pc.y + y][pc.x + x] = cell;
	return;
}

static bool collides(struct tetris_game_piece pc, unsigned char board[20][10])
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

static bool line_full(unsigned char board[20][10], unsigned char y)
{
	for (unsigned char x = 0; x < 10; x++) {
		if (board[y][x] == 0)
			return 0;
	}
	return 1;
}

static void place_piece_down(struct tetris_game *tgptr)
{
	for (unsigned char y = 0; y < 4; y++) {
		for (unsigned char x = 0; x < 4; x++) {
			if (cell_out_of_bounds(tgptr->pc, x, y))
				continue;
			set_cell(tgptr->pc, tgptr->board, x, y);
		}
	}
}

static void new_piece(struct tetris_game *tgptr)
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
	seed_random();

	struct tetris_game *tgptr = malloc(sizeof(struct tetris_game));
	tgptr->npcidx = rand() % 7;
	new_piece(tgptr);

	for (unsigned char y = 0; y < 20; y++) {
		for (unsigned char x = 0; x < 10; x++) {
			tgptr->board[y][x] = 0;
		}
	}

	tgptr->lines = 0;
	tgptr->score = 0;

	return tgptr;
}

struct tetris_game_result tetris_game_update(struct tetris_game *tgptr)
{
	struct tetris_game_result result;
	result.game_ended = 0;
	result.piece_dropped = 0;
	result.lines_cleared[0] = 0xFF;

	struct tetris_game_piece nextpc = tgptr->pc;
	nextpc.y++;

	if (!collides(nextpc, tgptr->board)) {
		tgptr->pc = nextpc;
		return result;
	}

	place_piece_down(tgptr);
	result.piece_dropped = 1;

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

struct tetris_game_result tetris_game_input(struct tetris_game *tgptr,
					    enum tetris_game_input input)
{
	struct tetris_game_result result;
	result.game_ended = 0;
	result.lines_cleared[0] = 0xFF;

	struct tetris_game_piece nextpc = tgptr->pc;
	switch (input) {
	case MOVE_LEFT:
		nextpc.x--;
		break;
	case MOVE_RIGHT:
		nextpc.x++;
		break;
	case SOFT_DROP:
		nextpc.y++;
		break;
	case HARD_DROP:
		struct tetris_game_result r = tetris_game_update(tgptr);
		while (!r.piece_dropped) {
			r = tetris_game_update(tgptr);
		}
		return r;
	case ROTATE_CW:
		nextpc.rot = (nextpc.rot + 1) % 4;
		break;
	case ROTATE_CCW:
		nextpc.rot = (nextpc.rot + 3) % 4;
		break;
	default:
		return result;
	}
	
	if (!collides(nextpc, tgptr->board))
		tgptr->pc = nextpc;
	return result;
}