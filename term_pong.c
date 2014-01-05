#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <ncurses.h>

#define DISPLAY_COLS 80
#define DISPLAY_ROWS 24

#define PADDLE_WIDTH 4

#define WHITE 1
#define BLACK 2

#define PADDLE_A_UP 'w'
#define PADDLE_A_DOWN 's'
#define PADDLE_B_UP 'i'
#define PADDLE_B_DOWN 'k'

void int_handler() {
    endwin();
    exit(0);
}

typedef struct game_state {
    int score_a;
    int score_b;
    int paddle_a_y;
    int paddle_b_y;
    int paddle_a_vel;
    int paddle_b_vel;
    int puck_x;
    int puck_y;
    int puck_vel_x;
    int puck_vel_y;
} game_state_t;

void init(game_state_t *state)
{
    state->score_a = 0;
    state->score_b = 0;
    state->paddle_a_y = (DISPLAY_ROWS - PADDLE_WIDTH) / 2;
    state->paddle_b_y = (DISPLAY_ROWS - PADDLE_WIDTH) / 2;
    state->paddle_a_vel = 0;
    state->paddle_b_vel = 0;
    state->puck_x = DISPLAY_COLS / 2 - 1;
    state->puck_y = DISPLAY_ROWS / 2 - 1;
    state->puck_vel_x = 1;
    state->puck_vel_y = 0;
}

void display_edge(int paddle_y, int row)
{
       if (row >= paddle_y && row < paddle_y + PADDLE_WIDTH) {
            attron(COLOR_PAIR(WHITE));
            printw(" ");
            attron(COLOR_PAIR(BLACK));
        } else {
            printw(" ");
        }

}

void display(game_state_t *state)
{
    int i, j;
    attron(COLOR_PAIR(2));
    for (i = 0; i < DISPLAY_ROWS; i++) {
        display_edge(state->paddle_a_y, i);
        for (j = 1; j < DISPLAY_COLS - 1; j++) {
            if (state->puck_x == j && state->puck_y == i) {
                attron(COLOR_PAIR(WHITE));
                printw(" ");
                attron(COLOR_PAIR(BLACK));
            } else {
                printw(" ");
            }
        }
        display_edge(state->paddle_b_y, i);
        printw("\n");
    }
    printw("Player a: %d\t\tPlayer b: %d\n", state->score_a, state->score_b);
    printw("Controls: w/s\t\t i/k\t\t ctr-c to quit\n");
}

void update(game_state_t *state)
{
    if (state->puck_x == 0) {
        state->score_b++;
        state->puck_x = DISPLAY_COLS / 2;
    }
    if (state->puck_x == DISPLAY_COLS - 1) {
        state->score_a++;
        state->puck_x = DISPLAY_COLS / 2;
    }
    if (state->puck_x == 1) {
        if (state->puck_y >= state->paddle_a_y && state->puck_y < state->paddle_a_y + PADDLE_WIDTH) {
            state->puck_vel_x = 1;
            if (state->paddle_a_vel)
                state->puck_vel_y = state->paddle_a_vel;
        }
    }
    if (state->puck_x == DISPLAY_COLS - 2) {
        if (state->puck_y >= state->paddle_b_y && state->puck_y < state->paddle_b_y + PADDLE_WIDTH)
            state->puck_vel_x = -1;
            if (state->paddle_b_vel)
                state->puck_vel_y = state->paddle_b_vel;
    }
    if (state->puck_y == 0 || state->puck_y == DISPLAY_ROWS - 1) {
        state->puck_vel_y = -state->puck_vel_y;
    }
    state->puck_x += state->puck_vel_x;
    state->puck_y += state->puck_vel_y;
    state->paddle_a_vel = 0;
    state->paddle_b_vel = 0;
}

void handle_input(game_state_t *state, char input)
{
    switch (input) {
        case PADDLE_A_UP:
            if (state->paddle_a_y > 0)
                state->paddle_a_y--;
            state->paddle_a_vel = -1;
            break;
        case PADDLE_A_DOWN:
            if (state->paddle_a_y < DISPLAY_ROWS - PADDLE_WIDTH)
                state->paddle_a_y++;
            state->paddle_a_vel = 1;
            break;
        case PADDLE_B_UP:
            state->paddle_b_vel = -1;
            if (state->paddle_b_y > 0)
                state->paddle_b_y--;
            break;
        case PADDLE_B_DOWN:
            state->paddle_b_vel = 1;
            if (state->paddle_b_y < DISPLAY_ROWS - PADDLE_WIDTH)
                state->paddle_b_y++;
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    fd_set fds;
    struct timeval timeout;
    game_state_t state;

    signal(SIGINT, int_handler);

    init(&state);

    initscr();
    start_color();
    init_pair(WHITE, COLOR_BLACK, COLOR_WHITE);
    init_pair(BLACK, COLOR_WHITE, COLOR_BLACK);

    timeout.tv_sec = 0;
    while (1) {
        clear();
        display(&state);
        refresh();

        while (timeout.tv_usec > 0) {
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);
            select(2, &fds, NULL, NULL, &timeout);
            if (FD_ISSET(STDIN_FILENO, &fds)) {
                handle_input(&state, getch());
            }
        }
        timeout.tv_usec = 100000;

        update(&state);
    }

    return 0;
}
