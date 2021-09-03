#include <malloc.h>
#include <curses.h>
#include <unistd.h>
#include "tetris.h"
#include <menu.h>

typedef struct {
    char *title;
    void *usr_ptr;
} t_menu_item;

void init_ncurses();

void destroy_ncurses();

void init_windows();

void t_print_rules(WINDOW *win);

void game();

void restart_game(tetris_game *game);

void stop_game(tetris_game *game);

void show_menu(tetris_game *game);

void msleep(int milliseconds);

int running = 1;
int game_over = 0;
int paused = 0;
WINDOW *tw_main;
WINDOW *tw_next;
WINDOW *tw_score;

int main() {
    init_ncurses();
    init_windows();
    game();
    destroy_ncurses();
    return 0;
}

void game() {
    tetris_game *t_game = t_init();
    tetris_block *t_block = t_init_block();
    int ticks = 0;

    t_reset_block(t_block, t_game);
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
            case ' ':
                paused = !paused;
                break;
            case 'q':
                return;
            default:
                break;
        }
        if (paused) {
            mvwprintw(tw_main, NUM_OF_ROWS / 2 - 1, NUM_OF_COLS * COLS_PER_ROW / 2 - 2, "PAUSE");
            wrefresh(tw_main);
            continue;
        }
        if (game_over) {
            show_menu(t_game);
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
    t_print_rules(tw_rules);
    wrefresh(tw_rules);
}

void show_menu(tetris_game *game) {
    int active = 1;
    nodelay(stdscr, FALSE);
    t_menu_item m_items[] = {
        {" play again ", restart_game},
        {"    exit    ", stop_game}
    };
    WINDOW *tw_menu = newwin(4, 14, NUM_OF_ROWS / 2 - 4, NUM_OF_COLS * COLS_PER_ROW / 2 - 6);
    int n_menu_items = sizeof(m_items) / sizeof(m_items[0]);
    ITEM **items = (ITEM **) calloc(n_menu_items + 1, sizeof(ITEM *));
    for (int i = 0; i < n_menu_items; ++i) {
        items[i] = new_item(m_items[i].title, NULL);
        set_item_userptr(items[i], m_items[i].usr_ptr);
    }
    items[n_menu_items] = (ITEM *) NULL;
    MENU *menu = new_menu(items);
    keypad(tw_menu, TRUE);
    set_menu_win(menu, tw_menu);
    set_menu_sub(menu, derwin(tw_menu, n_menu_items, 12, 1, 1));
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
                ITEM *cur;
                cur = current_item(menu);
                void (*p)(tetris_game *);
                p = item_userptr(cur);
                p(game);
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

void stop_game(tetris_game *game) {
    running = 0;
}

void restart_game(tetris_game *game) {
    t_reset_game(game);
    game_over = 0;
    running = 1;
}

void t_print_rules(WINDOW *win) {
    mvwprintw(win, 1, 1, "q: quit");
    mvwprintw(win, 2, 1, "space: pause");
}

void msleep(int milliseconds) {
    usleep(milliseconds * 1000);
}
