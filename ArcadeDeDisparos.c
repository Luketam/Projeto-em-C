#include <stdio.h>
#include <unistd.h>
#include "timer.h"
#include "screen.h"
#include "keyboard.h"

// Função principal
int main() {
    screenInit(1); // Inicializa a tela com bordas

    int playerX = 20;
    int isShot = 0;
    int shotX, shotY;

    keyboardInit(); // Inicializa a captura do teclado

    timerInit(50); // Inicializa um temporizador de 50 milissegundos

    while (1) {
        screenClear(); // Limpa a tela

        printf("\033[%d;%dH^", 24, playerX); // Desenha a nave

        if (isShot) {
            printf("\033[%d;%dH|", shotY, shotX); // Desenha o tiro
            shotY--;
            if (shotY <= 0) {
                isShot = 0;
            }
        }

        screenUpdate(); // Atualiza a tela

        usleep(50000); // Pausa para controlar a velocidade do jogo

        if (keyhit()) {
            char input = readch();
            if (input == 'a' && playerX > 1) {
                playerX--;
            } else if (input == 'd' && playerX < 40) {
                playerX++;
            } else if (input == ' ' && !isShot) {
                isShot = 1;
                shotX = playerX;
                shotY = 23;
            }
        }

        if (timerTimeOver()) {
            // Lógica de movimento dos inimigos ou outras atualizações do jogo a cada 50 ms
            timerUpdateTimer(50); // Reinicia o temporizador
        }
    }

    screenDestroy(); // Restaura as configurações da tela
    keyboardDestroy(); // Encerra a captura do teclado
    timerDestroy(); // Encerra o temporizador

    return 0;
}
