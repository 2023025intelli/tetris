#include <curses.h>
#include <unistd.h>
#include "tetris.h"
#include <menu.h>

#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio.h"

typedef struct {
    char *title;
    void *usr_ptr;
} t_menu_item;

void init_ncurses();

void destroy_ncurses();

void init_windows();

void print_rules(WINDOW *win);

void game(const char *sf);

void restart_game(tetris_game *game, tetris_block *block);

void unpause_game(tetris_game *game, tetris_block *block);

void stop_game(tetris_game *game, tetris_block *block);

void show_menu(tetris_game *game, tetris_block *block, t_menu_item *m_items, int items_n);

void msleep(int milliseconds);

void play_music(const char *filename);

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount);

int running = 1;
int game_over = 0;
int paused = 0;
WINDOW *tw_main;
WINDOW *tw_next;
WINDOW *tw_score;

ma_decoder decoder;
ma_device device;

int main(int argc, char *argv[]) {
    init_ncurses();
    init_windows();
    play_music("tetris.mp3");
    if (argc > 1) game(argv[1]);
    else game(NULL);
    destroy_ncurses();
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
    return 0;
}

void game(const char *sf) {
    tetris_game *t_game = t_init();
    tetris_block *t_block = t_init_block();
    int ticks = 0;
    int m_gameover_items_n = 2;
    t_menu_item m_gameover_items[] = {
        {" play again ", restart_game},
        {"    exit    ", stop_game}
    };
    int m_pause_items_n = 3;
    t_menu_item m_pause_items[] = {
        {"   resume   ", unpause_game},
        {" play again ", restart_game},
        {" save  game ", t_save_game},
        {"    exit    ", stop_game}
    };

    if (sf) { t_load_game(t_game, t_block); }
    else { t_reset_block(t_block, t_game); }
    t_draw_block(tw_main, t_block);
    t_draw_next_block(tw_next, t_game);
    wrefresh(tw_main);

    while (running) {
        switch (getch()) {
            case KEY_LEFT:
                t_move_block_left(t_block, t_game);
                break;
            case KEY_RIGHT:
                t_move_block_right(t_block, t_game);
                break;
            case KEY_UP:
                t_rotate_block(t_block, t_game);
                break;
            case KEY_DOWN:
                t_increase_speed(t_game);
                break;
            case 's':
                t_save_game(t_game, t_block);
                break;
            case ' ':
                paused = !paused;
                break;
            case 'q':
                return;
            default:
                break;
        }
        if (paused) {
            show_menu(t_game, t_block, m_pause_items, m_pause_items_n);
        }
        if (game_over) {
            show_menu(t_game, t_block, m_gameover_items, m_gameover_items_n);
        }
        ticks++;
        if (ticks >= t_game->tick_period / SLEEP_PERIOD) {
            ticks = 0;
            if (t_block_can_move(t_block, t_game)) {
                t_stop_block(t_block, t_game);
                t_check_board(t_game);
                if (t_is_game_over(t_game)) {
                    game_over = 1;
                    continue;
                }
                t_reset_block(t_block, t_game);
                t_draw_next_block(tw_next, t_game);
                t_normalize_speed(t_game);
            } else {
                t_tick_block(t_block);
            }
            mvwprintw(tw_score, 0, 0, "score: %d", t_game->score);
            mvwprintw(tw_score, 1, 0, "level: %d", t_game->level);
            wnoutrefresh(tw_score);
        }
        t_draw_board(tw_main, t_game);
        t_draw_block(tw_main, t_block);
        wnoutrefresh(tw_main);
        doupdate();
        msleep(SLEEP_PERIOD);
    }
    t_destroy_block(t_block);
    t_destroy(t_game);
}

void init_ncurses() {
    initscr();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    start_color();
}

void destroy_ncurses() {
    nocbreak();
    wclear(stdscr);
    endwin();
}

void init_windows() {
    wclear(stdscr);
    WINDOW *tw_main_border = newwin(22, 22, 0, 0);
    WINDOW *tw_next_border = newwin(7, 16, 0, 22);
    WINDOW *tw_score_border = newwin(7, 16, 7, 22);
    WINDOW *tw_rules = newwin(8, 16, 14, 22);
    tw_main = newwin(20, 20, 1, 1);
    tw_next = newwin(5, 14, 1, 23);
    tw_score = newwin(5, 14, 8, 23);
    refresh();
    box(tw_main_border, 0, 0);
    box(tw_next_border, 0, 0);
    box(tw_score_border, 0, 0);
    box(tw_rules, 0, 0);
    wrefresh(tw_main_border);
    wrefresh(tw_next_border);
    wrefresh(tw_score_border);
    wrefresh(tw_main);
    wrefresh(tw_next);
    wrefresh(tw_score);
    print_rules(tw_rules);
    wrefresh(tw_rules);
}

void show_menu(tetris_game *game, tetris_block *block, t_menu_item *m_items, int items_n) {
    int active = 1;
    nodelay(stdscr, FALSE);
    WINDOW *tw_menu = newwin(items_n + 2, 14, NUM_OF_ROWS / 2 - 4, NUM_OF_COLS * COLS_PER_ROW / 2 - 6);
    ITEM **items = (ITEM **) calloc(items_n + 1, sizeof(ITEM *));
    for (int i = 0; i < items_n; ++i) {
        items[i] = new_item(m_items[i].title, NULL);
        set_item_userptr(items[i], m_items[i].usr_ptr);
    }
    items[items_n] = (ITEM *) NULL;
    MENU *menu = new_menu(items);
    keypad(tw_menu, TRUE);
    set_menu_win(menu, tw_menu);
    set_menu_sub(menu, derwin(tw_menu, items_n, 12, 1, 1));
    set_menu_mark(menu, "");
    box(tw_menu, 0, 0);
    refresh();
    post_menu(menu);
    wrefresh(tw_menu);
    int c;
    while (active && (c = getch())) {
        switch (c) {
            case KEY_DOWN:
                menu_driver(menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(menu, REQ_UP_ITEM);
                break;
            case 10: { // Enter key pressed
                ITEM *cur = current_item(menu);
                ((void (*)(tetris_game *, tetris_block *block)) item_userptr(cur))(game, block);
                pos_menu_cursor(menu);
                active = 0;
                break;
            }
            default:
                break;
        }
        wrefresh(tw_menu);
    }
    unpost_menu(menu);
    free_item(items[0]);
    free_item(items[1]);
    free_menu(menu);
    delwin(tw_menu);
    nodelay(stdscr, TRUE);
}

void restart_game(tetris_game *game, tetris_block *block) {
    t_reset_game(game);
    t_reset_block(block, game);
    t_draw_next_block(tw_next, game);
    game_over = 0;
    paused = 0;
    running = 1;
}

void unpause_game(tetris_game *game, tetris_block *block) {
    paused = 0;
}

void stop_game(tetris_game *game, tetris_block *block) {
    running = 0;
}

void print_rules(WINDOW *win) {
    mvwprintw(win, 1, 1, "space: pause");
    mvwprintw(win, 2, 1, "s: save game");
    mvwprintw(win, 3, 1, "q: quit");
}

void msleep(int milliseconds) {
    usleep(milliseconds * 1000);
}

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount) {
    ma_bool32 isLooping = MA_TRUE;
    ma_decoder *pDecoder = (ma_decoder *) pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }
    ma_data_source_read_pcm_frames(pDecoder, pOutput, frameCount, NULL, isLooping);
    (void) pInput;
}

void play_music(const char *filename) {
    ma_result result;
    ma_device_config deviceConfig;

    result = ma_decoder_init_file(filename, NULL, &decoder);
    if (result != MA_SUCCESS) { return; }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate = decoder.outputSampleRate;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = &decoder;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        ma_decoder_uninit(&decoder);
        return;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return;
    }
}