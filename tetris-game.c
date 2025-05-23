#include "tetris-game.h"
#include <stdlib.h>
#include <time.h>

static const unsigned char EMPTY_BOARD[20][10] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static inline void seed_random()
{
	srand(time(NULL) + clock());
}

static inline unsigned char cell_out_of_bounds(struct tetris_game_piece pc,
					       unsigned char x, unsigned char y)
{
	return PIECE_DATA[pc.idx][pc.rot][y][x] != 0 &&
	       (pc.x + x >= 10 || pc.x + x < 0 || pc.y + y >= 20 ||
		pc.y + y < 0);
}

static inline unsigned char cell_overlaps(struct tetris_game_piece pc,
					  unsigned char (*board)[20][10],
					  unsigned char x, unsigned char y)
{
	return PIECE_DATA[pc.idx][pc.rot][y][x] != 0 &&
	       (*board)[pc.y + y][pc.x + x] != 0;
}

static inline void set_cell(struct tetris_game_piece pc,
			    unsigned char (*board)[20][10], unsigned char x,
			    unsigned char y)
{
	unsigned char cell = PIECE_DATA[pc.idx][pc.rot][y][x];
	if (cell == 0)
		return;
	(*board)[pc.y + y][pc.x + x] = cell;
	return;
}

static unsigned char collides(struct tetris_game_piece pc,
			      unsigned char (*board)[20][10])
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

static unsigned char line_full(unsigned char (*board)[20][10], unsigned char y)
{
	if (y > 19)
		return 0;
	for (unsigned char x = 0; x < 10; x++) {
		if ((*board)[y][x] == 0)
			return 0;
	}
	return 1;
}

static void clear_line(unsigned char (*board)[20][10], unsigned char y) {
	for (unsigned char i = y; i > 0; i--) {
		for (unsigned char x = 0; x < 10; x++) {
			(*board)[i][x] = (*board)[i - 1][x];
		}
	}
	for (unsigned char x = 0; x < 10; x++) {
		(*board)[0][x] = 0;
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

void copy_board(unsigned char (*dest)[20][10],
		const unsigned char (*src)[20][10])
{
	for (unsigned char y = 0; y < 20; y++) {
		for (unsigned char x = 0; x < 10; x++) {
			(*dest)[y][x] = (*src)[y][x];
		}
	}
}

void place_piece_down(unsigned char (*board)[20][10],
		      struct tetris_game_piece pc)
{
	for (unsigned char y = 0; y < 4; y++) {
		for (unsigned char x = 0; x < 4; x++) {
			if (cell_out_of_bounds(pc, x, y))
				continue;
			set_cell(pc, board, x, y);
		}
	}
}

struct tetris_game *new_tetris_game()
{
	seed_random();

	struct tetris_game *tgptr = malloc(sizeof(struct tetris_game));
	tgptr->npcidx = rand() % 7;
	new_piece(tgptr);

	copy_board(&tgptr->board, &EMPTY_BOARD);

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

	if (!collides(nextpc, &tgptr->board)) {
		tgptr->pc = nextpc;
		return result;
	}

	place_piece_down(&tgptr->board, tgptr->pc);
	result.piece_dropped = 1;

	unsigned char size = 0;
	for (unsigned char y = tgptr->pc.y; y < tgptr->pc.y + 4; y++) {
		if (line_full(&tgptr->board, y)) {
			clear_line(&tgptr->board, y);
			result.lines_cleared[size] = y;
			if (++size < 4)
				result.lines_cleared[size] = 0xFF;
		}
	}
	tgptr->score += CLEAR_REWARDS[size] * ((tgptr->lines / 10) + 1);
	tgptr->lines += size;

	new_piece(tgptr);

	result.game_ended = collides(tgptr->pc, &tgptr->board);

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
		result.piece_dropped = 0;
		while (!result.piece_dropped) {
			result = tetris_game_update(tgptr);
		}
		return result;
	case ROTATE_CW:
		nextpc.rot = (nextpc.rot + 1) % 4;
		break;
	case ROTATE_CCW:
		nextpc.rot = (nextpc.rot + 3) % 4;
		break;
	default:
		return result;
	}

	if (!collides(nextpc, &tgptr->board))
		tgptr->pc = nextpc;
	return result;
}