#include "keyboard.h"
#include "screen.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ENEMY_BULLET_SPEED 1
#define ENEMY_SHOOT_INTERVAL 100
#define HEART "\u2764"
#define INITIAL_PLAYER_LIFE 5
#define MAX_ENEMY_BULLETS 20

#define SHIP_WIDTH 5
#define SHIP_HEIGHT 1

#define KEY_LEFT 'a' 
#define KEY_RIGHT 'd' 

#define ENEMY_WIDTH 3
#define ENEMY_HEIGHT 1
#define MAX_ENEMIES 5

#define BULLET_WIDTH 1
#define BULLET_HEIGHT 1
#define MAX_BULLETS 10

typedef enum { ENEMY_TYPE_A, ENEMY_TYPE_B, ENEMY_TYPE_C } EnemyType;

typedef struct {
  int active;
  int x, y;
} EnemyBullet;

typedef struct {
  int x, y;
  int life;
  int active;
  EnemyType type; // Tipo de inimigo
} Enemy;

typedef struct {
  int active;
  int x, y;
} Bullet;

typedef struct {
  int x, y;
} Object;

typedef struct {
  int level;
  int totalEnemies;
} GameInfo;

EnemyBullet enemyBullets[MAX_ENEMY_BULLETS];
GameInfo gameInfo;
Object ship;
Enemy enemies[MAX_ENEMIES];
Bullet bullets[MAX_BULLETS];

int playerLife=5;
int direction=1; // Direção inicial (1 para a direita, -1 para a esquerda)
int score=0;     // Variável para pontuação

void initializeEnemyBullets() {
  for (int i = 0; i < MAX_ENEMY_BULLETS; ++i) {
    enemyBullets[i].active = 0;
    enemyBullets[i].x = -1;
    enemyBullets[i].y = -1;
  }
}

void drawEnemyBullets() {
  for (int i = 0; i < MAX_ENEMY_BULLETS; ++i) {
    if (enemyBullets[i].active) {
      screenGotoxy(enemyBullets[i].x, enemyBullets[i].y);
      printf("*");
    }
  }
}
void moveEnemyBullets() {
  for (int i = 0; i < MAX_ENEMY_BULLETS; ++i) {
    if (enemyBullets[i].active) {
      enemyBullets[i].y += ENEMY_BULLET_SPEED;
      if (enemyBullets[i].y >= MAXY) {
        enemyBullets[i].active = 0;
      }
    }
  }
}
void enemyShoot() {
  for (int i = 0; i < MAX_ENEMIES; ++i) {
    if (enemies[i].active) {
      for (int j = 0; j < MAX_ENEMY_BULLETS; ++j) {
        if (!enemyBullets[j].active) {
          enemyBullets[j].active = 1;
          enemyBullets[j].x = enemies[i].x + ENEMY_WIDTH / 2;
          enemyBullets[j].y = enemies[i].y + ENEMY_HEIGHT + 1;
          break;
        }
      }
    }
  }
}
void drawBorders() {
  screenBoxEnable();
  screenGotoxy(MINX, MINY);
  printf("%c", BOX_UPLEFT);

  for (int i = MINX + 1; i < MAXX; ++i) {
    screenGotoxy(i, MINY);
    printf("%c", BOX_HLINE);
  }

  screenGotoxy(MAXX, MINY);
  printf("%c", BOX_UPRIGHT);

  for (int i = MINY + 1; i < MAXY; ++i) {
    screenGotoxy(MINX, i);
    printf("%c", BOX_VLINE);
    screenGotoxy(MAXX, i);
    printf("%c", BOX_VLINE);
  }

  screenGotoxy(MINX, MAXY);
  printf("%c", BOX_DWNLEFT);

  for (int i = MINX + 1; i < MAXX; ++i) {
    screenGotoxy(i, MAXY);
    printf("%c", BOX_HLINE);
  }

  screenGotoxy(MAXX, MAXY);
  printf("%c", BOX_DWNRIGHT);
  screenBoxDisable();
}

void drawPlayerLife() {
  screenGotoxy(MINX + 1, MINY + 1);
  printf("Lives: ");
  for (int i = 0; i < playerLife; ++i) {
    printf(HEART);
    printf(" ");
  }
}

void initializeGame(int level) {
  gameInfo.level = level;
  gameInfo.totalEnemies = MAX_ENEMIES * (level + 1);

  ship.x = (MAXX - SHIP_WIDTH) / 2;
  ship.y = MAXY - SHIP_HEIGHT - 1;

  for (int i = 0; i < MAX_ENEMIES; ++i) {
    enemies[i].active = 0; // Desativa todos os inimigos antes de reativar para o novo nível
  }

  // Configuração dos inimigos para o nível 1 (tipo A)
  if (level == 1) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
      enemies[i].x = i * (ENEMY_WIDTH + 2) + 1;
      enemies[i].y = 2;
      enemies[i].type = ENEMY_TYPE_A;
      enemies[i].active = 1;
    }
  }
  // Configuração dos inimigos para o nível 2 (tipo B)
  else if (level == 2) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
      enemies[i].x = i * (ENEMY_WIDTH + 4) + 1;
      enemies[i].y = 2;
      enemies[i].type = ENEMY_TYPE_B;
      enemies[i].active = 1;
    }
  }
  // Configuração dos inimigos para o nível 3 (tipo C)
  else if (level == 3) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
      enemies[i].x = i * (ENEMY_WIDTH + 6) + 1;
      enemies[i].y = 2;
      enemies[i].type = ENEMY_TYPE_C;
      enemies[i].active = 1;
    }
  }
  for (int i = 0; i < MAX_BULLETS; ++i) {
    bullets[i].active = 0;
    bullets[i].x = -1;
    bullets[i].y = -1;
  }
  for (int i = 0; i < MAX_ENEMY_BULLETS; ++i) {
    enemyBullets[i].active = 0;
    enemyBullets[i].x = -1;
    enemyBullets[i].y = -1;
  }
}

int allEnemiesDefeated() {
  for (int i = 0; i < MAX_ENEMIES; ++i) {
    if (enemies[i].active) {
      return 0;
    }
  }
  return 1;
}

int activeEnemies() {
  int count = 0;
  for (int i = 0; i < MAX_ENEMIES; ++i) {
    if (enemies[i].active) {
      count++;
    }
  }
  return count;
}

void drawShip() {
  screenGotoxy(ship.x, ship.y);
  printf("⢀⡴⣿⢦⡀");
  drawPlayerLife(); // Desenha as vidas do jogador
}

void drawEnemies() {
  for (int i = 0; i < MAX_ENEMIES; ++i) {
    if (enemies[i].active) {
      screenGotoxy(enemies[i].x, enemies[i].y);
      switch (enemies[i].type) {
      case ENEMY_TYPE_A:
        printf("⢈⢝⠭⡫⡁");
        break;
      case ENEMY_TYPE_B:
        printf("⢘⠟⠛⠛⢟");
        break;
      case ENEMY_TYPE_C:
        printf("⢀⡴⣾⢿⡿⣷⢦⡀");
        break;
      default:
        printf("⢈⢝⠭⡫⡁");
        break;
      }
    }
  }
}

void drawBullets() {
  for (int i = 0; i < MAX_BULLETS; ++i) {
    if (bullets[i].active) {
      screenGotoxy(bullets[i].x, bullets[i].y);
      printf("|");
    }
  }
}

void moveShip(int direction) {
  ship.x += direction;
  if (ship.x < MINX) {
    ship.x = MINX;
  } else if (ship.x + SHIP_WIDTH > MAXX) {
    ship.x = MAXX - SHIP_WIDTH;
  }
}

void moveEnemies() {
  static int count = 0; // Variável estática para controlar o movimento vertical
  count++;

  for (int i = 0; i < MAX_ENEMIES; ++i) {
    if (enemies[i].active) {
      enemies[i].x += direction;
      if (count % 20 == 0) { // Muda a direção vertical a cada 20 iterações
        enemies[i].y += direction;
      }

      if (enemies[i].x + ENEMY_WIDTH >= MAXX || enemies[i].x <= MINX) {
        direction *= -1;
        for (int j = 0; j < MAX_ENEMIES; ++j) {
          enemies[j].y += 2; // Movimento vertical após mudar de direção
        }
        break;
      }
    }
  }
}


void moveBullets() {
  for (int i = 0; i < MAX_BULLETS; ++i) {
    if (bullets[i].active) {
      bullets[i].y--;
      if (bullets[i].y <= MINY) {
        bullets[i].active = 0;
      }
    }
  }
}

void handleCollisions() {
  // Verifica se as balas atingiram os inimigos
  for (int i = 0; i < MAX_ENEMIES; ++i) {
    if (enemies[i].active) {
      for (int j = 0; j < MAX_BULLETS; ++j) {
        if (bullets[j].active && bullets[j].y == enemies[i].y &&
            bullets[j].x >= enemies[i].x &&
            bullets[j].x <= enemies[i].x + ENEMY_WIDTH) {
          enemies[i].life--;
          bullets[j].active = 0;
          if (enemies[i].life <= 0) {
            enemies[i].active = 0;
            score += 100;
          }
        }
      }
    }
  }

  // Verifica se as balas atingiram a nave do jogador
  for (int i = 0; i < MAX_BULLETS; ++i) {
    if (bullets[i].active && bullets[i].y == ship.y && bullets[i].x >= ship.x &&
        bullets[i].x <= ship.x + SHIP_WIDTH) {
      bullets[i].active = 0;
      playerLife--; // Reduz a vida do jogador quando atingido
      if (playerLife <= 0) {
        // Game Over logic here
      }
    }
  }
}

void handleEnemyBulletCollisions() {
  for (int i = 0; i < MAX_ENEMY_BULLETS; ++i) {
    if (enemyBullets[i].active &&
        enemyBullets[i].y == ship.y &&
        enemyBullets[i].x >= ship.x &&
        enemyBullets[i].x <= ship.x + SHIP_WIDTH) {
      playerLife--;
      enemyBullets[i].active = 0;
      if (playerLife <= 0) {
        // Game Over logic here
        screenClear();
        screenGotoxy(MAXX / 2 - 4, MAXY / 2);
        printf("GAME OVER");
        screenUpdate();
        sleep(3); // Aguarda 2 segundos antes de encerrar o jogo
        break;
      }
    }
  }
}

void shoot() {
  for (int i = 0; i < MAX_BULLETS; ++i) {
    if (!bullets[i].active) {
      bullets[i].active = 1;
      bullets[i].x = ship.x + SHIP_WIDTH / 2;
      bullets[i].y = ship.y - 1;
      break;
    }
  }
}

int main() {
  screenInit(0);
  keyboardInit();
  timerInit(50);

  int level = 1;

  while (level <= 3) {
    initializeGame(level);
    initializeEnemyBullets();
    playerLife = INITIAL_PLAYER_LIFE; // Reinicia a vida do jogador

    int ch = 0;
    int enemyShootTimer = 0;

    while (ch != 'q') {
      if (activeEnemies() == 0) {
        screenClear();
        screenGotoxy(MAXX / 2 - 4, MAXY / 2);
        printf("You win level %d!", level);
        screenUpdate();
        sleep(2);

        if (level < 3) {
          level++;
          initializeGame(level);
          initializeEnemyBullets();
          playerLife = INITIAL_PLAYER_LIFE; // Reinicia a vida do jogador para o próximo nível
        } else {
          screenClear();
          screenGotoxy(MAXX / 2 - 7, MAXY / 2);
          printf("Congratulations!");
          screenGotoxy(MAXX / 2 - 9, MAXY / 2 + 1);
          printf("You completed all levels!");
          screenUpdate();
          sleep(2);
          ch = 'q'; // Termina o jogo após os níveis serem concluídos
        }
      }

      drawBorders();

      if (timerTimeOver()) {
        moveEnemies();
        moveBullets();
        moveEnemyBullets();
        handleCollisions();
        handleEnemyBulletCollisions();
        screenClear();
        drawShip();
        drawEnemies();
        drawBullets();
        drawEnemyBullets();
        screenGotoxy(MAXX - 15, MINY + 1);
        printf("Score: %d", score);
        screenUpdate();
      }

      enemyShootTimer++;
      if (enemyShootTimer >= 1000) {
        enemyShoot();
        enemyShootTimer = 0;
      }

      if (keyhit()) {
        ch = readch();
        if (ch == KEY_LEFT)
          moveShip(-1);
        else if (ch == KEY_RIGHT)
          moveShip(1);
        else if (ch == ' ') {
          shoot();
        }
      }

      if (playerLife <= 0) {
        screenClear();
        screenGotoxy(MAXX / 2 - 4, MAXY / 2);
        printf("GAME OVER");
        screenUpdate();
        sleep(3);
        ch = 'q'; // Termina o jogo após o game over
      }
    }

    if (ch=='q') {
      break;
    }
  }

  keyboardDestroy();
  screenDestroy();
  timerDestroy();

  return 0;
}
