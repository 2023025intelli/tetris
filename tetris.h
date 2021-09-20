#ifndef TETRIS_TETRIS_H
#define TETRIS_TETRIS_H

#define COLS_PER_ROW 2
#define NUM_OF_SHAPES 7
#define NUM_OF_ORIENTATIONS 4
#define NUM_OF_CELLS 4
#define NUM_OF_ROWS 20
#define NUM_OF_COLS 10
#define NUM_OF_COLORS 7
#define TICK_PERIOD 650
#define SLEEP_PERIOD 10
#define SCORE_PER_LINE 10
#define LEVEL_MODIFIER 10
#define LEVEL_PERIOD 30
#define MAX_LEVEL 10

typedef enum {
    BC_NONE = 0,
    BC_RED = 1,
    BC_WHITE = 2,
    BC_BLUE = 3,
    BC_GREEN = 4,
    BC_YELLOW = 5,
    BC_MAGENTA = 6,
    BC_CYAN = 7
} tetris_color;

typedef struct {
    int y;
    int x;
} tetris_cell;

typedef struct {
    int y;
    int x;
    int shape;
    int orientation;
    tetris_color color;
} tetris_block;

typedef struct {
    int **board;
    int level;
    int score;
    int tick_period;
    int next_shape;
    int next_orientation;
    tetris_color next_color;
} tetris_game;

tetris_game *t_init();

void t_reset_game(tetris_game *game);

void t_init_colors();

void t_destroy(tetris_game *game);

int t_random_shape();

int t_random_orientation();

int t_random_color();

tetris_block *t_init_block();

void t_reset_block(tetris_block *block, tetris_game *game);

void t_draw_block(WINDOW *win, tetris_block *block);

void t_draw_next_block(WINDOW *win, tetris_game *game);

void t_draw_board(WINDOW *win, tetris_game *game);

void t_move_block_left(tetris_block *block, tetris_game *game);

void t_move_block_right(tetris_block *block, tetris_game *game);

void t_rotate_block(tetris_block *block, tetris_game *game);

int t_block_can_move(tetris_block *block, tetris_game *game);

int t_is_block_fits(tetris_block *block, int orientation, tetris_game *game);

void t_tick_block(tetris_block *block);

void t_stop_block(tetris_block *block, tetris_game *game);

void t_increase_speed(tetris_game *game);

void t_normalize_speed(tetris_game *game);

void t_check_board(tetris_game *game);

void t_check_level(tetris_game *game);

int t_is_game_over(tetris_game *game);

void t_destroy_block(tetris_block *block);

void t_save_game(tetris_game *game, tetris_block *block);

void t_load_game(tetris_game *game, tetris_block *block);

#endif //TETRIS_TETRIS_H

