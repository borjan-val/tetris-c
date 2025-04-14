#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

static const unsigned char PIECE_DATA[7][4][4][4] = {
	{ { { 0, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 0, 0 } },
	  { { 1, 1, 1, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 0, 0 } },
	  { { 1, 1, 1, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } } },
	{ { { 0, 0, 2, 0 }, { 0, 0, 2, 0 }, { 0, 2, 2, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 0, 0, 0 }, { 0, 2, 0, 0 }, { 0, 2, 2, 2 }, { 0, 0, 0, 0 } },
	  { { 0, 2, 2, 0 }, { 0, 2, 0, 0 }, { 0, 2, 0, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 0, 0, 0 }, { 2, 2, 2, 0 }, { 0, 0, 2, 0 }, { 0, 0, 0, 0 } } },
	{ { { 0, 3, 0, 0 }, { 0, 3, 0, 0 }, { 0, 3, 3, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 0, 0, 0 }, { 0, 3, 3, 3 }, { 0, 3, 0, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 3, 3, 0 }, { 0, 0, 3, 0 }, { 0, 0, 3, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 0, 0, 0 }, { 0, 0, 3, 0 }, { 3, 3, 3, 0 }, { 0, 0, 0, 0 } } },
	{ { { 0, 4, 4, 0 }, { 0, 4, 4, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 4, 4, 0 }, { 0, 4, 4, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 4, 4, 0 }, { 0, 4, 4, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 4, 4, 0 }, { 0, 4, 4, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } } },
	{ { { 0, 5, 0, 0 }, { 0, 5, 5, 0 }, { 0, 0, 5, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 0, 5, 5 }, { 0, 5, 5, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 5, 0, 0 }, { 0, 5, 5, 0 }, { 0, 0, 5, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 0, 5, 5 }, { 0, 5, 5, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } } },
	{ { { 0, 0, 0, 0 }, { 6, 6, 6, 0 }, { 0, 6, 0, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 6, 0, 0 }, { 6, 6, 0, 0 }, { 0, 6, 0, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 6, 0, 0 }, { 6, 6, 6, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 6, 0, 0 }, { 0, 6, 6, 0 }, { 0, 6, 0, 0 }, { 0, 0, 0, 0 } } },
	{ { { 0, 0, 7, 0 }, { 0, 7, 7, 0 }, { 0, 7, 0, 0 }, { 0, 0, 0, 0 } },
	  { { 7, 7, 0, 0 }, { 0, 7, 7, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	  { { 0, 0, 7, 0 }, { 0, 7, 7, 0 }, { 0, 7, 0, 0 }, { 0, 0, 0, 0 } },
	  { { 7, 7, 0, 0 }, { 0, 7, 7, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } } }
};

static const unsigned short CLEAR_REWARDS[5] = { 0, 40, 100, 300, 1200 };

// Store state of current piece
struct tetris_game_piece {
	char x;
	char y;
	unsigned char idx;
	unsigned char rot;
};

// Store the state of a tetris game
struct tetris_game {
	unsigned char board[20][10];
	struct tetris_game_piece pc;
	unsigned char npcidx;
	unsigned short lines;
	unsigned int score;
};

// Store info about the result of a game step or input
struct tetris_game_result {
	bool game_ended;
	unsigned char lines_cleared[4];
};

// Possible inputs for a game
enum tetris_game_input {
	MOVE_LEFT,
	MOVE_RIGHT,
	SOFT_DROP,
	HARD_DROP,
	ROTATE_CW,
	ROTATE_CCW
};

// Create a new Tetris game
struct tetris_game *new_tetris_game();

// Perform a game step
struct tetris_game_result tetris_game_update(struct tetris_game *tgptr);

// Pass an input to a Tetris game
struct tetris_game_result tetris_game_input(struct tetris_game *tgptr,
					    enum tetris_game_input input);

#endif