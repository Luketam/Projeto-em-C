#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
//$ gcc ./src/*.c -I./include -o cli-lib-example ---pra baixar no linux---
#define MAX_ENEMIES 10

typedef struct {
    int x, y;
} Player;

typedef struct {
    int x, y;
    int active;
} Enemy;

void drawPlayer(Player *player) {
    mvprintw(player->y, player->x, "A");
}

void drawEnemy(Enemy *enemy) {
    if (enemy->active) {
        mvprintw(enemy->y, enemy->x, "X");
    }
}

void moveEnemies(Enemy *enemies) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (enemies[i].active) {
            enemies[i].y++;
            if (enemies[i].y >= LINES) {
                enemies[i].active = 0;
            }
        }
    }
}

int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(100);

    Player player = {COLS / 2, LINES - 1};
    Enemy *enemies = malloc(MAX_ENEMIES * sizeof(Enemy));

    for (int i = 0; i < MAX_ENEMIES; ++i) {
        enemies[i].x = rand() % COLS;
        enemies[i].y = 0;
        enemies[i].active = 0;
    }

    int ch;
    int score = 0;

    while ((ch = getch()) != 'q') {
        clear();

        switch (ch) {
            case KEY_LEFT:
                player.x--;
                break;
            case KEY_RIGHT:
                player.x++;
                break;
            case ' ':
                for (int i = 0; i < MAX_ENEMIES; ++i) {
                    if (!enemies[i].active) {
                        enemies[i].x = player.x;
                        enemies[i].y = player.y - 1;
                        enemies[i].active = 1;
                        break;
                    }
                }
                break;
        }

        moveEnemies(enemies);

        drawPlayer(&player);
        for (int i = 0; i < MAX_ENEMIES; ++i) {
            drawEnemy(&enemies[i]);
        }

        // Check for collisions
        for (int i = 0; i < MAX_ENEMIES; ++i) {
            if (enemies[i].active && enemies[i].x == player.x && enemies[i].y == player.y) {
                mvprintw(LINES / 2, COLS / 2 - 5, "Game Over!");
                refresh();
                getch(); // Wait for user input
                endwin();
                free(enemies);
                return 0;
            }
        }

        refresh();
    }

    endwin();
    free(enemies);
    return 0;
}