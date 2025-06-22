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

// Seed the pRNG with a new time-based integer
static inline void seed_random()
{
	srand(time(NULL) + clock());
}

// Check whether the given piece has a non-empty cell outside of the game board
// at the given relative coordinates
static inline unsigned char cell_out_of_bounds(struct tetris_game_piece pc,
					       unsigned char x, unsigned char y)
{
	return PIECE_DATA[pc.idx][pc.rot][y][x] != 0 &&
	       (pc.x + x >= 10 || pc.x + x < 0 || pc.y + y >= 20 ||
		pc.y + y < 0);
}

// Check whether both the given piece and the given game board have a non-empty
// cell at the given coordinates
static inline unsigned char cell_overlaps(struct tetris_game_piece pc,
					  unsigned char (*board)[20][10],
					  unsigned char x, unsigned char y)
{
	return PIECE_DATA[pc.idx][pc.rot][y][x] != 0 &&
	       (*board)[pc.y + y][pc.x + x] != 0;
}

// Set a game board cell corresponding to a set of relative coordinates and a
// piece to the corresponding piece cell if that piece cell is not empty
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

// Check whether a piece collides with the given game board
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

// Check whether all cells on a given line of a given board are non-empty
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

// Do a "line clear" by shifting down the lines above and setting the topmost
// line to all empty cells
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

// Replace the current piece of a Tetris game with the next piece, position it
// at the top of the board in default rotation and generate a new random
// "next piece" to succeed the new current piece.
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
	// Prepare a result struct to be returned
	struct tetris_game_result result;
	result.game_ended = 0;
	result.piece_dropped = 0;
	result.lines_cleared[0] = 0xFF;

	// Check whether the current piece can simply fall downwards one cell
	struct tetris_game_piece nextpc = tgptr->pc;
	nextpc.y++;

	if (!collides(nextpc, &tgptr->board)) {
		tgptr->pc = nextpc;
		return result;
	}

	// The piece can't fall; it needs to be "dropped" and the game needs to
	// move on to the next piece
	place_piece_down(&tgptr->board, tgptr->pc);
	result.piece_dropped = 1;

	// Check if dropping the piece resulted in any lines being filled and
	// clear them
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

	// We're done placing down the current piece, get a new one
	new_piece(tgptr);

	// If a piece is already in an invalid position when entering the
	// game, the game has ended
	result.game_ended = collides(tgptr->pc, &tgptr->board);

	return result;
}

struct tetris_game_result tetris_game_input(struct tetris_game *tgptr,
					    enum tetris_game_input input)
{
	struct tetris_game_result result;
	result.game_ended = 0;
	result.lines_cleared[0] = 0xFF;

	// Depending on the input, the current piece will end up in a different
	// position; create a copy of the current piece and check whether it
	// would collide or end up in a valid position (with the exception of
	// a hard drop)
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
		// Drop the piece as far as it will go - no further action is
		// required afterwards, return immediately
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
		// If the input is invalid, we don't move our copy of the
		// current piece and thus don't even need to bother checking
		// its collision, we can return immediately
		return result;
	}

	// Check if our piece copy ends up in a valid position and apply our
	// changes to the real piece if that is the case
	if (!collides(nextpc, &tgptr->board))
		tgptr->pc = nextpc;
	return result;
}