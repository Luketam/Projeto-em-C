#include "keyboard.h"
#include "screen.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VEL_TIROS_INIMIGOS 1 //velocidade dos tiros
#define INTERVALO_TIRO 100 //intervalo dos tiros em ms
#define CORACAO "\u2764" //simbolo de coracao com unicode
#define VIDA_INICIAL 3 //vida inicial do jgdr
#define MAX_TIROS_INIMIGOS 20 //qnts tiros os inimigos tem no max
#define LNAVE 5 //largura da nave
#define ANAVE 1 //altura. usados p calcular colisoes
#define ESQUERDA 'a' 
#define DIREITA 'd' 
#define LINIMIGO 3 //msm coisa da nave.
#define AINIMIGO 1
#define MAX_INIMIGOS 5 //o max de inimigos
#define LTIRO 1 //largura e altura dos tiros p colisoes
#define ATIRO 1
#define MAX_TIROS 6 //max de balas ativas do jgdr

enum TipoInimigo {INIMIGO_UNICO};

typedef struct Node {
    char nome[4];
    int pontos;
    struct Node *next;
} Node;

Node *listaPontuacao = NULL;

typedef struct{ //definicao das balas inimigas e suas coordenadas (x, y)
  int Iniciado; 
  int x, y;
} TiroDosInimigos;
typedef struct { 
    int x, y;
    int vida;
    int Iniciado;
    enum TipoInimigo Tipo; // Adicionei o membro Tipo à estrutura Enemy
} Enemy;
typedef struct{ //balas do jgdr e suas coordenadas
  int Iniciado;
  int x, y;
} Bullet;
typedef struct{ //outros objetos e suas coordenadas
  int x, y;
} Objeto;
typedef struct{ //info do jg, como os niveis e total de inimigos
  int Nivel;
  int TotalInimigos;
} InfoDoJG;

TiroDosInimigos TirosInimigos[MAX_TIROS_INIMIGOS]; //array com tam max do total de tiros inimigos
InfoDoJG InfoJG; //guarda info do jg
Objeto Nave; //representa a nave do jgdr
Enemy Inimigos[MAX_INIMIGOS]; //array com tam max do total de inimigos
Bullet Tiros[MAX_TIROS]; //array com tam max do total de tiros
int VidaJgdr=3;
int Direcao=1; //1/positivo p direita, -1/negativo p esquerda
int Score=0;

void adicionarPontuacao(char nome[4], int pontos) {
    Node *novoNode = (Node *)malloc(sizeof(Node));
    strcpy(novoNode->nome, nome);
    novoNode->pontos = pontos;
    novoNode->next = NULL;

    if (listaPontuacao == NULL) {
        listaPontuacao = novoNode;
    } else {
        Node *atual = listaPontuacao;
        Node *anterior = NULL;

        while (atual != NULL && pontos < atual->pontos) {
            anterior = atual;
            atual = atual->next;
        }

        if (anterior == NULL) {
            novoNode->next = listaPontuacao;
            listaPontuacao = novoNode;
        } else {
            anterior->next = novoNode;
            novoNode->next = atual;
        }
    }
}
void mostrarRanking() {
    printf("\nRanking:\n");

    Node *atual = listaPontuacao;
    int posicao = 1;

    while (atual != NULL && posicao <= 3) {
        printf("%d. %s - %d pontos\n", posicao, atual->nome, atual->pontos);
        atual = atual->next;
        posicao++;
    }
}
void salvarPontuacaoEmArquivo() {
    FILE *arquivo = fopen("pontuacao.dat", "wb");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo para escrita.\n");
        exit(EXIT_FAILURE);
    }

    Node *atual = listaPontuacao;

    while (atual != NULL) {
        fwrite(atual, sizeof(Node), 1, arquivo);
        atual = atual->next;
    }

    fclose(arquivo);
}

void carregarPontuacaoDoArquivo() {
    FILE *arquivo = fopen("pontuacao.dat", "rb");
    if (arquivo != NULL) {
        Node buffer;

        while (fread(&buffer, sizeof(Node), 1, arquivo) == 1) {
            adicionarPontuacao(buffer.nome, buffer.pontos);
        }

        fclose(arquivo);
    }
}
void liberarListaPontuacao() {
    Node *atual = listaPontuacao;
    while (atual != NULL) {
        Node *proximo = atual->next;
        free(atual);
        atual = proximo;
    }
}

void IniciarTirosInimigos(){
  for (int i=0; i<MAX_TIROS_INIMIGOS; ++i){ // garante q os tiros dos inimigos estejam desativados
    TirosInimigos[i].Iniciado = 0;              //até a inicialização destes
    TirosInimigos[i].x = -1;
    TirosInimigos[i].y = -1;
  }
}
void DesignTirosInimigos(){
  for (int i=0; i<MAX_TIROS_INIMIGOS; ++i){  //percorre os tiros dos inimigos ativos e os desenha na tela (x, y)
    if (TirosInimigos[i].Iniciado){              
      screenGotoxy(TirosInimigos[i].x, TirosInimigos[i].y);
      printf("\033[1;33m!\033m");
    }
  }
}
void PercursoTirosInimigos(){
  for (int i=0; i<MAX_TIROS_INIMIGOS; ++i){ //faz a animação dos tiros e desativa os q atingem a parte inferior da tela
    if (TirosInimigos[i].Iniciado){
      TirosInimigos[i].y += VEL_TIROS_INIMIGOS;
      if (TirosInimigos[i].y >= MAXY){
        TirosInimigos[i].Iniciado = 0;
      }
    }
  }
}
void TiroInimigo(){
  for (int i = 0; i < MAX_INIMIGOS; ++i){ // verifica os inimigos na tela e ativa os tiros q n estao em uso
    if (Inimigos[i].Iniciado){
      for (int j = 0; j < MAX_TIROS_INIMIGOS; ++j){
        if (!TirosInimigos[j].Iniciado){
          TirosInimigos[j].Iniciado = 1;
          TirosInimigos[j].x = Inimigos[i].x + LINIMIGO / 2;
          TirosInimigos[j].y = Inimigos[i].y + AINIMIGO + 1;
          break;
        }
      }
    }
  }
}
void drawBorders(){ //funcao da biblioteca de tiaguinho
  screenSetColor(CYAN, BLACK);
  screenBoxEnable();
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
void DesignVidaJgdr(){  //desenha na tela a vida do jgdr e a ajusta, conforme ele perde vida
  screenGotoxy(MINX + 1, MINY + 1);
  printf("  \033[1;34mVida: \033[m");
  for (int i=0; i<VidaJgdr; ++i) {
    printf("\033[0;31m%s\033[m", CORACAO); //unicode
    printf(" ");
  }
}
void IniciarJogo(int Nivel) {
  InfoJG.Nivel = Nivel;
  InfoJG.TotalInimigos = MAX_INIMIGOS * (Nivel + 1);
  Nave.x = (MAXX - LNAVE) / 2;
  Nave.y = MAXY - ANAVE - 1;

  for (int i = 0; i < MAX_INIMIGOS; ++i) {
    Inimigos[i].Iniciado = 0; // Desativa todos os inimigos antes de reativar para o novo nível
  }
  for (int i = 0; i < MAX_INIMIGOS; ++i) {
    Inimigos[i].x = i * (LINIMIGO + 4) + 1; // Posicionamento linear dos inimigos
    Inimigos[i].y = 2;
    Inimigos[i].Iniciado = 1;
  }
  for (int i = 0; i < MAX_TIROS; ++i) {
    Tiros[i].Iniciado = 0;
    Tiros[i].x = -1;
    Tiros[i].y = -1;
  }
  for (int i = 0; i < MAX_TIROS_INIMIGOS; ++i) {
    TirosInimigos[i].Iniciado = 0;
    TirosInimigos[i].x = -1;
    TirosInimigos[i].y = -1;
  }
}
int InimigosDerrotados(){ //percorre o array de inimigos e verifica se tem algum ativo (Inimigos[i].Iniciado)
  for (int i=0; i<MAX_INIMIGOS; ++i){ //se encontrar, retorna 0, se nao, retorna 1
    if (Inimigos[i].Iniciado){
      return 0;
    }
  }
  return 1;
}
int IniciadoInimigos(){
  int count = 0;
  for (int i=0; i<MAX_INIMIGOS; ++i){ //percorre o array de inimigos e restorna a quantidade deles que estao em campo
    if (Inimigos[i].Iniciado){
      count++;
    }
  }
  return count;
}
void DesignNave(){ //desenha na tela a nave do jgdr (x, y)
  screenGotoxy(Nave.x, Nave.y);
  printf("\033[1;34m⢀⡴⣿⢦⡀\033[m");
  DesignVidaJgdr();
}
void DesignInimigos() {
    for (int i = 0; i < MAX_INIMIGOS; ++i) {
        if (Inimigos[i].Iniciado && Inimigos[i].Tipo == INIMIGO_UNICO) {
            screenGotoxy(Inimigos[i].x, Inimigos[i].y);
            printf("\033[1;33m⢈⢝⠭⡫⡁\033[m");
            // Aqui você desenha o inimigo de acordo com suas coordenadas (Inimigos[i].x, Inimigos[i].y)
        }
    }
}
void DesignTiros(){ //desenha os tiros do jgdr
  for (int i=0; i<MAX_TIROS; ++i){
    if (Tiros[i].Iniciado){
      screenGotoxy(Tiros[i].x, Tiros[i].y);
      printf("\033[1;36m|\033[m");
    }
  }
}
void PercursoNave(int movimento){ //mantem a nave dentro dos limites da tela (movimentacao horizontal)
  Nave.x += movimento;
  if (Nave.x < MINX){
    Nave.x = MINX;
  } else if (Nave.x + LNAVE > MAXX){
    Nave.x = MAXX - LNAVE;
  }
}
void PercursoInimigos(){ //movimentacao dos inimigos
  static int count = 0; //usada para controlar o movimento horizontal
  count++; //a cada chamada da função, o valor de count é incrementado
  for (int i=0; i<MAX_INIMIGOS; ++i){ 
    if (Inimigos[i].Iniciado){ //verifica se o inimigo está iniciado
      Inimigos[i].x += Direcao; //move p esquerda ou direita
      if (count % 10 == 0){ //sendo múltiplo de 10, se movem verticalmente
        Inimigos[i].y += Direcao;
      }
      if (Inimigos[i].x + LINIMIGO >= MAXX || Inimigos[i].x <= MINX){ //verifica se algum inimigo atingiu as bordas horizontais da tela
        Direcao *= -1; // Inverte a direção horizontal
        for (int j=0; j<MAX_INIMIGOS; ++j){ // Move todos os inimigos para baixo após a inversao
          Inimigos[j].y += 1; // Movimento vertical após mudar de direção
        }
      }
    }
  }
}
void PercursoTiros(){
  for (int i=0; i<MAX_TIROS; ++i){
    if (Tiros[i].Iniciado){
      Tiros[i].y--; //simula o movimento de subida
      if (Tiros[i].y <= MINY){ //verifica se o tiro esta ativo na tela ou n
        Tiros[i].Iniciado = 0; 
      }
    }
  }
}
void Colisoes(){
  for (int i=0; i<MAX_INIMIGOS; ++i){ // Verifica se as balas atingiram os inimigos
    if (Inimigos[i].Iniciado){
      for (int j=0; j<MAX_TIROS; ++j){ //verifica se a posicao dos tiros do jgdr e da nave inimiga coincidem
        if (Tiros[j].Iniciado && Tiros[j].y == Inimigos[i].y &&
            Tiros[j].x >= Inimigos[i].x &&
            Tiros[j].x <= Inimigos[i].x + LINIMIGO){
          Inimigos[i].vida--; //Se um tiro atingir um inimigo, ele morre 
          Tiros[j].Iniciado = 0; //seu tiro é desativado
          if (Inimigos[i].vida <= 0){ //inimigo morto, ent é desativado
            Inimigos[i].Iniciado = 0;
            Score += 100; 
          }
        }
      }
    }
  }
  for (int i=0; i<MAX_TIROS; ++i){ // Verifica se as balas atingiram a nave do jogador
    if (Tiros[i].Iniciado && Tiros[i].y == Nave.y && Tiros[i].x >= Nave.x && Tiros[i].x <= Nave.x + LNAVE){ //verifica se estao na msm coordenada
      Tiros[i].Iniciado = 0; 
      VidaJgdr--; // Reduz a vida do jogador quando atingido
      if (VidaJgdr <= 0){
        printf("\033[1;31mGAME OVER\033[m");
        break;
      }
    }
  }
}
void Colisoes2(){ 
  for (int i=0; i<MAX_TIROS_INIMIGOS; ++i){ //percorre os tiros ativos p ver se atingiram o jgdr
    if (TirosInimigos[i].Iniciado && //tiro esta ativo?
        TirosInimigos[i].y == Nave.y && //verifica as coordenadas
        TirosInimigos[i].x >= Nave.x &&
        TirosInimigos[i].x <= Nave.x + LNAVE){
      VidaJgdr--; //se atingir, o jgdr perde vida
      TirosInimigos[i].Iniciado = 0;
      if (VidaJgdr <= 0){ //jgdr morto
        screenClear();
        screenGotoxy(MAXX / 2 - 4, MAXY / 2);
        printf("\033[1;31mGAME OVER\033[m");
        screenUpdate();
        sleep(3); // Aguarda  segundos antes de encerrar o jogo
        break;
      }
    }
  }
}
void Atirar(){ 
  for (int i=0; i<MAX_TIROS; ++i){ //percorre o array de tiros
    if (!Tiros[i].Iniciado){ //Verifica se um tiro específico não está iniciado
      Tiros[i].Iniciado = 1;
      Tiros[i].x = Nave.x + LNAVE / 2; //posicao do tiro especifico
      Tiros[i].y = Nave.y - 1;
      break;
    } //garante que so um tiro seja disparado por vez
  }
}
int main() {
  screenInit(0);
  keyboardInit();
  timerInit(50);
  int Nivel = 1;

  IniciarJogo(Nivel);
  IniciarTirosInimigos();
  VidaJgdr = VIDA_INICIAL;
  int ch = 0;
  int TiroInimigoTimer = 0;
  int jogadorDigitouNome = 0; // Nova variável para verificar se o jogador já digitou seu nome

  carregarPontuacaoDoArquivo(); // Carregar pontuações do arquivo antes de iniciar o jogo

  while (ch != 'q' && !jogadorDigitouNome) { // Verifica se o jogador não digitou o nome ainda
      drawBorders();

        if (timerTimeOver()) {
            PercursoInimigos();
            PercursoTiros();
            PercursoTirosInimigos();
            Colisoes();
            Colisoes2();
            screenClear();
            DesignNave();
            DesignTiros();
            DesignInimigos();
            DesignTirosInimigos();
            screenGotoxy(MAXX - 15, MINY + 1);
            printf("\033[1;34mScore:\033[m \033[1;34m%d\033[m", Score);
            screenUpdate();
        }

        if (InimigosDerrotados()) {
            screenClear();
            screenGotoxy(MAXX / 2 - 4, MAXY / 2);

            char nome[4];
            printf("\033[1;32mYOU WIN!\nDigite seu nome (3 caracteres): ");
            scanf("%3s", nome);

            adicionarPontuacao(nome, Score);
            mostrarRanking();
            salvarPontuacaoEmArquivo();

            jogadorDigitouNome = 1; // Define que o jogador digitou o nome
            screenUpdate();
            sleep(3);
    }

        TiroInimigoTimer++;
        if (TiroInimigoTimer >= 750) {
            TiroInimigo();
            TiroInimigoTimer = 0;
        }

        if (keyhit()) {
            ch = readch();
            if (ch == ESQUERDA)
                PercursoNave(-1);
            else if (ch == DIREITA)
                PercursoNave(1);
            else if (ch == ' ') {
                Atirar();
            }
        }

        if (VidaJgdr <= 0) {
            screenClear();
            screenGotoxy(MAXX / 2 - 4, MAXY / 2);
            screenUpdate();
            printf("\033[1;31mGAME OVER\033[m");
            sleep(3);
            ch = 'q';
        }
    }

  liberarListaPontuacao(); // Liberar memória alocada para a lista de pontuações
  keyboardDestroy();
  screenDestroy();
  timerDestroy();
  return 0;
}