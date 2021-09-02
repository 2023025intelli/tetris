#include <curses.h>
#include <malloc.h>
#include <stdlib.h>
#include "tetris.h"
#include <time.h>

int accumulate(int number);

tetris_cell SHAPES[NUM_OF_SHAPES][NUM_OF_ORIENTATIONS][NUM_OF_CELLS] = {
    // I
    {
        {{1, 0}, {1, 1}, {1, 2}, {1, 3}},
        {{0, 1}, {1, 1}, {2, 1}, {3, 1}},
        {{1, 0}, {1, 1}, {1, 2}, {1, 3}},
        {{0, 1}, {1, 1}, {2, 1}, {3, 1}}
    },
    // L
    {
        {{0, 1}, {1, 1}, {2, 1}, {2, 2}},
        {{0, 2}, {1, 0}, {1, 1}, {1, 2}},
        {{0, 0}, {0, 1}, {1, 1}, {2, 1}},
        {{1, 0}, {1, 1}, {1, 2}, {2, 0}}
    },
    // T
    {
        {{0, 1}, {1, 0}, {1, 1}, {1, 2}},
        {{0, 1}, {1, 0}, {1, 1}, {2, 1}},
        {{1, 0}, {1, 1}, {1, 2}, {2, 1}},
        {{0, 1}, {1, 1}, {1, 2}, {2, 1}}
    },
    // Z
    {
        {{1, 0}, {1, 1}, {2, 1}, {2, 2}},
        {{0, 2}, {1, 1}, {1, 2}, {2, 1}},
        {{1, 0}, {1, 1}, {2, 1}, {2, 2}},
        {{0, 2}, {1, 1}, {1, 2}, {2, 1}}
    },
    // O
    {
        {{0, 1}, {0, 2}, {1, 1}, {1, 2}},
        {{0, 1}, {0, 2}, {1, 1}, {1, 2}},
        {{0, 1}, {0, 2}, {1, 1}, {1, 2}},
        {{0, 1}, {0, 2}, {1, 1}, {1, 2}}
    },
    // J
    {
        {{0, 1}, {1, 1}, {2, 0}, {2, 1}},
        {{1, 0}, {1, 1}, {1, 2}, {2, 2}},
        {{0, 1}, {0, 2}, {1, 1}, {2, 1}},
        {{0, 0}, {1, 0}, {1, 1}, {1, 2}}
    },
    // S
    {
        {{1, 1}, {1, 2}, {2, 0}, {2, 1}},
        {{0, 1}, {1, 1}, {1, 2}, {2, 2}},
        {{1, 1}, {1, 2}, {2, 0}, {2, 1}},
        {{0, 1}, {1, 1}, {1, 2}, {2, 2}}
    }
};

tetris_game *t_init() {
    t_init_colors();
    srand(time(NULL));
    int **board;
    tetris_game *game = malloc(sizeof(tetris_game));
    board = malloc(sizeof(int *) * NUM_OF_ROWS);
    for (int i = 0; i < NUM_OF_ROWS; i++) {
        *(board + i) = malloc(sizeof(int) * NUM_OF_COLS);
    }
    for (int i = 0; i < NUM_OF_ROWS; i++) {
        for (int j = 0; j < NUM_OF_COLS; j++) {
            board[i][j] = BC_NONE;
        }
    }
    game->board = board;
    game->level = 1; // can not be 0
    game->score = 0;
    game->tick_period = TICK_PERIOD;
    game->next_shape = t_random_shape();
    game->next_orientation = t_random_orientation();
    game->next_color = t_random_color();
    return game;
}

void t_init_colors() {
    init_pair(BC_RED, COLOR_RED, COLOR_BLACK);
    init_pair(BC_WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(BC_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(BC_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(BC_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(BC_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(BC_CYAN, COLOR_CYAN, COLOR_BLACK);
}

void t_destroy(tetris_game *game) {
    for (int i = 0; i < NUM_OF_ROWS; i++) {
        free(*(game->board + i));
    }
    free(game->board);
    free(game);
}

int t_random_shape() {
    return rand() % NUM_OF_SHAPES;
}

int t_random_orientation() {
    return rand() % NUM_OF_ORIENTATIONS;
}

int t_random_color() {
    return (rand() % NUM_OF_COLORS) + 1;
}

// it is important for x value to be even...
tetris_block *t_init_block() {
    tetris_block *block = malloc(sizeof(tetris_block));
    return block;
}

void t_reset_block(tetris_block *block, tetris_game *game) {
    int shape = game->next_shape;
    int orientation = game->next_orientation;
    int color = game->next_color;
    game->next_shape = t_random_shape();
    game->next_orientation = t_random_orientation();
    game->next_color = t_random_color();
    tetris_cell *cells = SHAPES[shape][orientation];
    block->shape = shape;
    block->orientation = orientation;
    block->y = 0 - cells->y;
    block->x = (NUM_OF_COLS / 2 - 2) * COLS_PER_ROW;
    block->color = color;
}

void t_draw_block(WINDOW *win, tetris_block *block) {
    tetris_cell *cells = SHAPES[block->shape][block->orientation];
    wattr_on(win, COLOR_PAIR(block->color), NULL);
    for (int i = 0; i < NUM_OF_CELLS; i++) {
        for (int j = 0; j < COLS_PER_ROW; j++) {
            int y = block->y + (cells + i)->y;
            int x = block->x + (cells + i)->x * COLS_PER_ROW + j;
            mvwaddch(win, y, x, ACS_CKBOARD);
        }
    }
    wattr_off(win, COLOR_PAIR(block->color), NULL);
}

void t_draw_next_block(WINDOW *win, tetris_game *game) {
    wclear(win);
    tetris_cell *cells = SHAPES[game->next_shape][game->next_orientation];
    wattr_on(win, COLOR_PAIR(game->next_color), NULL);
    for (int i = 0; i < NUM_OF_CELLS; i++) {
        for (int j = 0; j < COLS_PER_ROW; j++) {
            int y = (cells + i)->y + 1;
            int x = (cells + i)->x * COLS_PER_ROW + j + COLS_PER_ROW;
            mvwaddch(win, y, x, ACS_CKBOARD);
        }
    }
    wattr_off(win, COLOR_PAIR(game->next_color), NULL);
    wnoutrefresh(win);
}

void t_draw_board(WINDOW *win, tetris_game *game) {
    for (int i = 0; i < NUM_OF_ROWS; i++) {
        for (int j = 0; j < NUM_OF_COLS; j++) {
            if (game->board[i][j]) {
                wattr_on(win, COLOR_PAIR(game->board[i][j]), NULL);
                for (int k = 0; k < COLS_PER_ROW; k++) {
                    int y = i;
                    int x = j * COLS_PER_ROW + k;
                    mvwaddch(win, y, x, ACS_CKBOARD);
                }
                wattr_off(win, COLOR_PAIR(game->board[i][j]), NULL);
            } else {
                for (int k = 0; k < COLS_PER_ROW; k++) {
                    int y = i;
                    int x = j * COLS_PER_ROW + k;
                    mvwaddch(win, y, x, ' ');
                }
            }
        }
    }
}

void t_move_block_left(tetris_block *block, tetris_game *game) {
    tetris_cell *cells = SHAPES[block->shape][block->orientation];
    for (int i = 0; i < NUM_OF_CELLS; i++) {
        int y = block->y + (cells + i)->y;
        int x = (block->x / COLS_PER_ROW) + (cells + i)->x - 1;
        if (x < 0 || game->board[y][x]) {
            return;
        }
    }
    block->x -= COLS_PER_ROW;
}

void t_move_block_right(tetris_block *block, tetris_game *game) {
    tetris_cell *cells = SHAPES[block->shape][block->orientation];
    for (int i = 0; i < NUM_OF_CELLS; i++) {
        int y = block->y + (cells + i)->y;
        int x = (block->x / COLS_PER_ROW) + (cells + i)->x + 1;
        if (x > NUM_OF_COLS - 1 || game->board[y][x]) {
            return;
        }
    }
    block->x += COLS_PER_ROW;
}

void t_rotate_block(tetris_block *block, tetris_game *game) {
    int orientation = (block->orientation + 1) % NUM_OF_ORIENTATIONS;
    if (t_is_block_fits(block, orientation, game)) {
        block->orientation = orientation;
        return;
    }
    block->x += COLS_PER_ROW;
    if (t_is_block_fits(block, orientation, game)) {
        block->orientation = orientation;
        return;
    }
    block->x -= 2 * COLS_PER_ROW;
    if (t_is_block_fits(block, orientation, game)) {
        block->orientation = orientation;
        return;
    }
    block->x += COLS_PER_ROW;
}

int t_is_block_fits(tetris_block *block, int orientation, tetris_game *game) {
    int fits = 1;
    tetris_cell *cells = SHAPES[block->shape][orientation];
    for (int i = 0; i < NUM_OF_CELLS; i++) {
        int y = block->y + (cells + i)->y;
        int x = (block->x / COLS_PER_ROW) + (cells + i)->x;
        if (y < 0 || x < 0 || game->board[y][x] || x > NUM_OF_COLS - 1 || y > NUM_OF_ROWS - 1) {
            fits = 0;
        }
    }
    return fits;
}

int t_block_can_move(tetris_block *block, tetris_game *game) {
    tetris_cell *cells = SHAPES[block->shape][block->orientation];
    for (int i = 0; i < NUM_OF_CELLS; i++) {
        int y = block->y + (cells + i)->y + 1;
        int x = (block->x / COLS_PER_ROW) + (cells + i)->x;
        if (y >= NUM_OF_ROWS || game->board[y][x]) {
            return 1;
        }
    }
    return 0;
}

void t_tick_block(tetris_block *block) {
    block->y++;
}

void t_stop_block(tetris_block *block, tetris_game *game) {
    tetris_cell *cells = SHAPES[block->shape][block->orientation];
    for (int i = 0; i < NUM_OF_CELLS; i++) {
        int y = block->y + (cells + i)->y;
        int x = (block->x / COLS_PER_ROW) + (cells + i)->x;
        game->board[y][x] = block->color;
    }
}

void t_check_board(tetris_game *game) {
    for (int i = NUM_OF_ROWS - 1; i >= 0; i--) {
        int full = 1;
        for (int j = 0; j < NUM_OF_COLS; j++) {
            if (!game->board[i][j]) { full = 0; }
        }
        if (full) {
            game->score += SCORE_PER_LINE;
            t_check_level(game);
            for (int j = 0; j < NUM_OF_COLS; j++) {
                game->board[i][j] = 0;
            }
            for (int j = i; j > 1; j--) {
                for (int k = 0; k < NUM_OF_COLS; k++) {
                    game->board[j][k] = game->board[j - 1][k];
                }
            }
            i++;
        }
    }
}

int t_is_game_over(tetris_game *game) {
    for (int i = 0; i < NUM_OF_COLS; i++) {
        if (game->board[0][i]) {
            return 1;
        }
    }
    return 0;
}

void t_check_level(tetris_game *game) {
    if (game->level >= MAX_LEVEL) {
        return;
    }
    if (game->score >= accumulate(game->level) * SCORE_PER_LINE * LEVEL_MODIFIER) {
        game->level++;
    }
}

void t_increase_speed(tetris_game *game) {
    game->tick_period = 50;
}

void t_normalize_speed(tetris_game *game) {
    game->tick_period = TICK_PERIOD - (game->level - 1) * LEVEL_PERIOD;
}

void t_destroy_block(tetris_block *block) {
    free(block);
}

int accumulate(int number) {
    if (number <= 0) { return 1; }
    int sum = 0;
    for (int i = 1; i <= number; i++) {
        sum += i;
    }
    return sum;
}