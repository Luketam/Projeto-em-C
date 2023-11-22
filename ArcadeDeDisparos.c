#include "keyboard.h"
#include "screen.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VEL_TIROS_INIMIGOS 1 
#define INTERVALO_TIRO 100 
#define CORACAO "\u2764" 
#define VIDA_INICIAL 3 
#define MAX_TIROS_INIMIGOS 20 
#define LNAVE 5
#define ANAVE 1 
#define ESQUERDA 'a' 
#define DIREITA 'd' 
#define LINIMIGO 3 
#define AINIMIGO 1
#define MAX_INIMIGOS 5 
#define LTIRO 1 
#define ATIRO 1
#define MAX_TIROS 6 

enum TipoInimigo {INIMIGO_UNICO};

typedef struct Node{
    char nome[4];
    int pontos;
    struct Node *next;
} Node;
typedef struct{
  int Iniciado; 
  int x, y;
} TiroDosInimigos;
typedef struct{ 
    int x, y;
    int vida;
    int Iniciado;
    enum TipoInimigo Tipo; 
} NavesInimigas;
typedef struct{ 
  int Iniciado;
  int x, y;
} Tiro;
typedef struct{ 
  int x, y;
} Objeto;
typedef struct{ 
  int Nivel;
  int TotalInimigos;
} InfoDoJG;

Node *listaPontuacao = NULL;
TiroDosInimigos TirosInimigos[MAX_TIROS_INIMIGOS];
InfoDoJG InfoJG; 
Objeto Nave; 
NavesInimigas Inimigos[MAX_INIMIGOS]; 
Tiro Tiros[MAX_TIROS]; 
int VidaJgdr=3;
int Direcao=1; 
int Score=0;

void adicionarPontuacao(char nome[4], int pontos){
    Node *novoNode = (Node *)malloc(sizeof(Node));
    strcpy(novoNode->nome, nome);
    novoNode->pontos = pontos;
    novoNode->next = NULL;
    if (listaPontuacao == NULL){
        listaPontuacao = novoNode;
    } else {
        Node *atual = listaPontuacao;
        Node *anterior = NULL;
        while (atual != NULL && pontos < atual->pontos) {
            anterior = atual;
            atual = atual->next;
        }
        if (anterior == NULL){
            novoNode->next = listaPontuacao;
            listaPontuacao = novoNode;
        } else{
            anterior->next = novoNode;
            novoNode->next = atual;
        }
    }
}
void mostrarRanking(){
    printf("\n\033[1;34mRanking:\033[m\n");
    Node *atual = listaPontuacao;
    int posicao = 1;

    while (atual != NULL && posicao <= 3) {
        printf("%d. %s \033[1;34m- %d pontos\033[m\n", posicao, atual->nome, atual->pontos);
        atual = atual->next;
        posicao++;
    }
}
void salvarPontuacaoEmArquivo(){
    FILE *arquivo = fopen("pontuacao.dat", "wb");
    if (arquivo == NULL) {
        exit(EXIT_FAILURE);
    }
    Node *atual = listaPontuacao;
    while (atual != NULL) {
        fwrite(atual, sizeof(Node), 1, arquivo);
        atual = atual->next;
    }
    fclose(arquivo);
}

void carregarPontuacaoDoArquivo(){
    FILE *arquivo = fopen("pontuacao.dat", "rb");
    if (arquivo != NULL) {
        Node buffer;
        while (fread(&buffer, sizeof(Node), 1, arquivo) == 1) {
            adicionarPontuacao(buffer.nome, buffer.pontos);
        }
        fclose(arquivo);
    }
}
void liberarListaPontuacao(){
    Node *atual = listaPontuacao;
    while (atual != NULL) {
        Node *proximo = atual->next;
        free(atual);
        atual = proximo;
    }
}
void IniciarTirosInimigos(){
  for (int i=0; i<MAX_TIROS_INIMIGOS; ++i){ 
    TirosInimigos[i].Iniciado = 0;             
    TirosInimigos[i].x = -1;
    TirosInimigos[i].y = -1;
  }
}
void DesignTirosInimigos(){
  for (int i=0; i<MAX_TIROS_INIMIGOS; ++i){  
    if (TirosInimigos[i].Iniciado){              
      screenGotoxy(TirosInimigos[i].x, TirosInimigos[i].y);
      printf("\033[1;33m!\033m");
    }
  }
}
void PercursoTirosInimigos(){
  for (int i=0; i<MAX_TIROS_INIMIGOS; ++i){ 
    if (TirosInimigos[i].Iniciado){
      TirosInimigos[i].y += VEL_TIROS_INIMIGOS;
      if (TirosInimigos[i].y >= MAXY){
        TirosInimigos[i].Iniciado = 0;
      }
    }
  }
}
void TiroInimigo(){
  for (int i = 0; i < MAX_INIMIGOS; ++i){ 
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
void drawBorders(){ 
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
void DesignVidaJgdr(){  
  screenGotoxy(MINX + 1, MINY + 1);
  printf("  \033[1;34mVida: \033[m");
  for (int i=0; i<VidaJgdr; ++i) {
    printf("\033[0;31m%s\033[m", CORACAO); 
    printf(" ");
  }
}
void IniciarJogo(int Nivel){
  InfoJG.Nivel = Nivel;
  InfoJG.TotalInimigos = MAX_INIMIGOS * (Nivel + 1);
  Nave.x = (MAXX - LNAVE) / 2;
  Nave.y = MAXY - ANAVE - 1;

  for (int i = 0; i < MAX_INIMIGOS; ++i) {
    Inimigos[i].Iniciado = 0; 
  }
  for (int i = 0; i < MAX_INIMIGOS; ++i) {
    Inimigos[i].x = i * (LINIMIGO + 4) + 1; 
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
int InimigosDerrotados(){ 
  for (int i=0; i<MAX_INIMIGOS; ++i){ 
    if (Inimigos[i].Iniciado){
      return 0;
    }
  }
  return 1;
}
int IniciadoInimigos(){
  int count = 0;
  for (int i=0; i<MAX_INIMIGOS; ++i){ 
    if (Inimigos[i].Iniciado){
      count++;
    }
  }
  return count;
}
void DesignNave(){ 
  screenGotoxy(Nave.x, Nave.y);
  printf("\033[1;34m⢀⡴⣿⢦⡀\033[m");
  DesignVidaJgdr();
}
void DesignInimigos(){
    for (int i = 0; i < MAX_INIMIGOS; ++i) {
        if (Inimigos[i].Iniciado && Inimigos[i].Tipo == INIMIGO_UNICO) {
            screenGotoxy(Inimigos[i].x, Inimigos[i].y);
            printf("\033[1;33m⢈⢝⠭⡫⡁\033[m");
        }
    }
}
void DesignTiros(){ 
  for (int i=0; i<MAX_TIROS; ++i){
    if (Tiros[i].Iniciado){
      screenGotoxy(Tiros[i].x, Tiros[i].y);
      printf("\033[1;36m|\033[m");
    }
  }
}
void PercursoNave(int movimento){ 
  Nave.x += movimento;
  if (Nave.x < MINX){
    Nave.x = MINX;
  } else if (Nave.x + LNAVE > MAXX){
    Nave.x = MAXX - LNAVE;
  }
}
void PercursoInimigos(){ 
  static int count = 0; 
  count++; 
  for (int i=0; i<MAX_INIMIGOS; ++i){ 
    if (Inimigos[i].Iniciado){
      Inimigos[i].x += Direcao; 
      if (count % 10 == 0){ 
        Inimigos[i].y += Direcao;
      }
      if (Inimigos[i].x + LINIMIGO >= MAXX || Inimigos[i].x <= MINX){ 
        Direcao *= -1; 
        for (int j=0; j<MAX_INIMIGOS; ++j){ 
          Inimigos[j].y += 1; 
        }
      }
    }
  }
}
void PercursoTiros(){
  for (int i=0; i<MAX_TIROS; ++i){
    if (Tiros[i].Iniciado){
      Tiros[i].y--; 
      if (Tiros[i].y <= MINY){ 
        Tiros[i].Iniciado = 0; 
      }
    }
  }
}
void Colisoes(){
  for (int i=0; i<MAX_INIMIGOS; ++i){ 
    if (Inimigos[i].Iniciado){
      for (int j=0; j<MAX_TIROS; ++j){ 
        if (Tiros[j].Iniciado && Tiros[j].y == Inimigos[i].y &&
            Tiros[j].x >= Inimigos[i].x &&
            Tiros[j].x <= Inimigos[i].x + LINIMIGO){
          Inimigos[i].vida--; 
          Tiros[j].Iniciado = 0; 
          if (Inimigos[i].vida <= 0){ 
            Inimigos[i].Iniciado = 0;
            Score += 100; 
          }
        }
      }
    }
  }
  for (int i=0; i<MAX_TIROS; ++i){ 
    if (Tiros[i].Iniciado && Tiros[i].y == Nave.y && Tiros[i].x >= Nave.x && Tiros[i].x <= Nave.x + LNAVE){ 
      Tiros[i].Iniciado = 0; 
      VidaJgdr--; 
      if (VidaJgdr <= 0){
        printf("\033[1;31mGAME OVER\033[m");
        break;
      }
    }
  }
}
void Colisoes2(){ 
  for (int i=0; i<MAX_TIROS_INIMIGOS; ++i){ 
    if (TirosInimigos[i].Iniciado && 
        TirosInimigos[i].y == Nave.y && 
        TirosInimigos[i].x >= Nave.x &&
        TirosInimigos[i].x <= Nave.x + LNAVE){
      VidaJgdr--;
      Score -= 50;
      TirosInimigos[i].Iniciado = 0;
      if (VidaJgdr <= 0){ 
        screenClear();
        screenGotoxy(MAXX / 2 - 4, MAXY / 2);
        printf("\033[1;31mGAME OVER\033[m");
        screenUpdate();
        sleep(3);
        exit(0); 
      }
    }
  }
  for (int i=0; i<MAX_INIMIGOS; ++i){
    if (Inimigos[i].Iniciado && Inimigos[i].y + AINIMIGO >= MAXY){
      screenClear();
      screenGotoxy(MAXX / 2 - 4, MAXY / 2);
      printf("\033[1;31mGAME OVER\033[m");
      screenUpdate();
      sleep(3);
      exit(0); 
    }
  }
}
void Atirar(){ 
  for (int i=0; i<MAX_TIROS; ++i){ 
    if (!Tiros[i].Iniciado){ 
      Tiros[i].Iniciado = 1;
      Tiros[i].x = Nave.x + LNAVE / 2; 
      Tiros[i].y = Nave.y - 1;
      break;
    } 
  }
}
int main(){
  screenInit(0);
  keyboardInit();
  timerInit(50);
  int Nivel = 1;

  IniciarJogo(Nivel);
  IniciarTirosInimigos();
  VidaJgdr = VIDA_INICIAL;
  int letra = 0;
  int TiroInimigoTimer = 0;
  int jogadorDigitouNome = 0; 

  carregarPontuacaoDoArquivo();
  while (letra != 's' && !jogadorDigitouNome) { 
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
        if (InimigosDerrotados()){
            screenClear();
            screenGotoxy(MAXX / 2 - 4, MAXY / 2);
            char nome[4];
            printf("\033[1;32mYOU WIN!\n\033[1;34mDigite seu nome (3 caracteres):\033[m ");
            scanf("%3s", nome);
            adicionarPontuacao(nome, Score);
            mostrarRanking();
            salvarPontuacaoEmArquivo();
            jogadorDigitouNome = 1; 
            screenUpdate();
            sleep(6);
        }
        TiroInimigoTimer++;
        if (TiroInimigoTimer >= 900) {
            TiroInimigo();
            TiroInimigoTimer = 0;
        }
        if (keyhit()){
            letra = readch();
            if (letra == ESQUERDA)
                PercursoNave(-1);
            else if (letra == DIREITA)
                PercursoNave(1);
            else if (letra == ' ') {
                Atirar();
            }
        }
        if (VidaJgdr <= 0){
            screenClear();
            screenGotoxy(MAXX / 2 - 4, MAXY / 2);
            screenUpdate();
            printf("\033[1;31mGAME OVER\033[m");
            sleep(3);
            letra = 's';
        }
    }
  liberarListaPontuacao(); 
  keyboardDestroy();
  screenDestroy();
  timerDestroy();
  return 0;
}