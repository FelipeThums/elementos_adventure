// Elemental Adventure - versao simples com sprites (Allegro 5)
// Usa allegro, allegro_primitives e allegro_image.
// As pastas Assets/ e assets_cave/ precisam estar na pasta do projeto (as mesmas do jogo completo).
//
// Controles: setas = andar | E = espada | Q = picareta | ESC = sair
// Entre na casa pela porta; a caverna fica na ponta esquerda do caminho.
// Objetivo: na caverna, quebrar os 6 minerios de ferro (picareta) e enfrentar os slimes (espada)

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

#define LARGURA 1280
#define ALTURA 720
#define NUM_SLIMES 4
#define NUM_MINERIOS 6
#define VIDA_MAX 5
#define TAM_MINERIO 56
#define ALCANCE_GOLPE 60
#define SLIME_W 32
#define SLIME_H 28
#define MAX_OBST 40
#define MAX_OBJ 40

// mapa (tiles de 16px desenhados em 48px)
#define TILE 48
#define CAMINHO_Y 384
#define LAGO_X 672
#define LAGO_Y 96
#define LAGO_COLS 6
#define LAGO_LINHAS 4
#define POSTE_ESQ_X 350
#define POSTE_DIR_X 880
#define POSTE_Y (CAMINHO_Y - 90)
#define BAU_X 520
#define BAU_Y 300

// caverna
#define CAV_PASTA "assets_cave/Objects_separately/"
#define CAV_NUM_IMGS 8
#define CAV_ROCHA_G 0
#define CAV_ROCHA_M 1
#define CAV_ROCHA_P 2
#define CAV_CRISTAL_M 3
#define CAV_CRISTAL_P 4
#define CAV_COGUMELO 5
#define CAV_FOGUEIRA 6
#define CAV_ESQUELETO 7

typedef struct {
    float x, y, w, h;
} Retangulo;

typedef struct {
    float x, y;
    int w, h;           // hitbox (o sprite e maior e e desenhado em volta dela)
    int vel;
    int vida;
    int dir;            // 0 baixo, 1 direita, 2 cima, 3 esquerda
    int invulneravel;   // contador de frames
    int tempo_ataque;   // frames restantes do golpe (so para animar)
    int ferramenta;     // 0 = espada, 1 = picareta
    int coluna, linha;  // frame da animacao
    int contador_anim;
} Jogador;

typedef struct {
    float x, y;
    int vida;
    int dir;            // 1 direita, -1 esquerda
    float vel;
    float lim_esq, lim_dir;
    int frame, contador;
    int dano;           // fica vermelho enquanto > 0
} Slime;

typedef struct {
    float x, y;
    int vida;
    int ativo;
} Minerio;

// qualquer coisa desenhada a partir de uma imagem (arvore, casa, movel...)
// base_y = linha do "pe" do objeto, para decidir se fica na frente ou atras do jogador
typedef struct {
    ALLEGRO_BITMAP* img;
    float sx, sy, sw, sh;
    float dx, dy, esc;
    float base_y;
} Objeto;

// imagens (globais para facilitar)
ALLEGRO_BITMAP* img_player;
ALLEGRO_BITMAP* img_player_acoes;
ALLEGRO_BITMAP* img_slime;
ALLEGRO_BITMAP* img_grama;
ALLEGRO_BITMAP* img_caminho;
ALLEGRO_BITMAP* img_agua;
ALLEGRO_BITMAP* img_decor;
ALLEGRO_BITMAP* img_casa;
ALLEGRO_BITMAP* img_arvore;
ALLEGRO_BITMAP* img_arvore_peq;
ALLEGRO_BITMAP* img_bau;
ALLEGRO_BITMAP* img_int_piso;
ALLEGRO_BITMAP* img_int_parede_topo;
ALLEGRO_BITMAP* img_int_parede_base;
ALLEGRO_BITMAP* img_int_capacho;
ALLEGRO_BITMAP* img_int_cama;
ALLEGRO_BITMAP* img_int_mesa;
ALLEGRO_BITMAP* img_int_escrivaninha;
ALLEGRO_BITMAP* img_int_tapete;
ALLEGRO_BITMAP* img_int_fornalha;
ALLEGRO_BITMAP* img_int_estante;
ALLEGRO_BITMAP* img_int_janela;
ALLEGRO_BITMAP* img_int_bau;
ALLEGRO_BITMAP* img_cav[CAV_NUM_IMGS];

// ---------- texto feito com retangulos (cada letra = 5 colunas x 7 linhas) ----------
// a ordem dos caracteres aqui tem que ser a mesma da tabela de baixo
const char* caracteres = " -():/!0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

const unsigned char letras[43][7] = {
    { 0,0,0,0,0,0,0 },                          // espaco
    { 0,0,0,0x1F,0,0,0 },                       // -
    { 0x02,0x04,0x08,0x08,0x08,0x04,0x02 },     // (
    { 0x08,0x04,0x02,0x02,0x02,0x04,0x08 },     // )
    { 0,0x0C,0x0C,0,0x0C,0x0C,0 },              // :
    { 0x01,0x01,0x02,0x04,0x08,0x10,0x10 },     // /
    { 0x04,0x04,0x04,0x04,0x04,0,0x04 },        // !
    { 0x0E,0x11,0x13,0x15,0x19,0x11,0x0E },     // 0
    { 0x04,0x0C,0x04,0x04,0x04,0x04,0x0E },     // 1
    { 0x0E,0x11,0x01,0x02,0x04,0x08,0x1F },     // 2
    { 0x1E,0x01,0x01,0x0E,0x01,0x01,0x1E },     // 3
    { 0x02,0x06,0x0A,0x12,0x1F,0x02,0x02 },     // 4
    { 0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E },     // 5
    { 0x06,0x08,0x10,0x1E,0x11,0x11,0x0E },     // 6
    { 0x1F,0x01,0x02,0x04,0x08,0x08,0x08 },     // 7
    { 0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E },     // 8
    { 0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C },     // 9
    { 0x0E,0x11,0x11,0x1F,0x11,0x11,0x11 },     // A
    { 0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E },     // B
    { 0x0E,0x11,0x10,0x10,0x10,0x11,0x0E },     // C
    { 0x1E,0x11,0x11,0x11,0x11,0x11,0x1E },     // D
    { 0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F },     // E
    { 0x1F,0x10,0x10,0x1E,0x10,0x10,0x10 },     // F
    { 0x0E,0x11,0x10,0x17,0x11,0x11,0x0F },     // G
    { 0x11,0x11,0x11,0x1F,0x11,0x11,0x11 },     // H
    { 0x0E,0x04,0x04,0x04,0x04,0x04,0x0E },     // I
    { 0x07,0x02,0x02,0x02,0x02,0x12,0x0C },     // J
    { 0x11,0x12,0x14,0x18,0x14,0x12,0x11 },     // K
    { 0x10,0x10,0x10,0x10,0x10,0x10,0x1F },     // L
    { 0x11,0x1B,0x15,0x15,0x11,0x11,0x11 },     // M
    { 0x11,0x19,0x15,0x13,0x11,0x11,0x11 },     // N
    { 0x0E,0x11,0x11,0x11,0x11,0x11,0x0E },     // O
    { 0x1E,0x11,0x11,0x1E,0x10,0x10,0x10 },     // P
    { 0x0E,0x11,0x11,0x11,0x15,0x12,0x0D },     // Q
    { 0x1E,0x11,0x11,0x1E,0x14,0x12,0x11 },     // R
    { 0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E },     // S
    { 0x1F,0x04,0x04,0x04,0x04,0x04,0x04 },     // T
    { 0x11,0x11,0x11,0x11,0x11,0x11,0x0E },     // U
    { 0x11,0x11,0x11,0x11,0x11,0x0A,0x04 },     // V
    { 0x11,0x11,0x11,0x15,0x15,0x1B,0x11 },     // W
    { 0x11,0x11,0x0A,0x04,0x0A,0x11,0x11 },     // X
    { 0x11,0x11,0x0A,0x04,0x04,0x04,0x04 },     // Y
    { 0x1F,0x01,0x02,0x04,0x08,0x10,0x1F }      // Z
};

// desenha um texto na tela. esc = tamanho de cada "pixel" da letra
// centro = 1 centraliza o texto em volta do x
void desenhar_texto(const char* texto, float x, float y, int esc, ALLEGRO_COLOR cor, int centro) {
    int i, linha, coluna;
    int tamanho = (int)strlen(texto);

    if (centro) {
        x = x - (tamanho * 6 * esc) / 2.0;
    }

    for (i = 0; i < tamanho; i++) {
        char c = toupper((unsigned char)texto[i]);
        char* pos = strchr(caracteres, c);

        if (pos != NULL) {
            int indice = pos - caracteres;
            for (linha = 0; linha < 7; linha++) {
                for (coluna = 0; coluna < 5; coluna++) {
                    if (letras[indice][linha] & (0x10 >> coluna)) {
                        float px = x + i * 6 * esc + coluna * esc;
                        float py = y + linha * esc;
                        al_draw_filled_rectangle(px, py, px + esc, py + esc, cor);
                    }
                }
            }
        }
    }
}

// ---------- FUNCOES BASICAS ----------

// testa se dois retangulos se encostam
int colide(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    if (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2) {
        return 1;
    }
    return 0;
}

// colisao so com os PES do jogador (faixa de 10 px na base). Assim a cabeca
// pode passar na frente de casas e arvores
int colide_pes(Jogador* p, Retangulo r) {
    return colide(p->x, p->y + p->h - 10, p->w, 10, r.x, r.y, r.w, r.h);
}

// area na frente do jogador onde o golpe acerta
Retangulo area_golpe(Jogador* p) {
    Retangulo r;
    if (p->dir == 0) { r.x = p->x;        r.y = p->y + p->h; r.w = p->w;        r.h = ALCANCE_GOLPE; }
    if (p->dir == 1) { r.x = p->x + p->w; r.y = p->y;        r.w = ALCANCE_GOLPE; r.h = p->h; }
    if (p->dir == 2) { r.x = p->x;        r.y = p->y - ALCANCE_GOLPE; r.w = p->w; r.h = ALCANCE_GOLPE; }
    if (p->dir == 3) { r.x = p->x - ALCANCE_GOLPE; r.y = p->y; r.w = ALCANCE_GOLPE; r.h = p->h; }
    return r;
}

// bate em algum obstaculo ou em minerio ainda inteiro?
int bateu(float x, float y, int w, int h, Retangulo obst[], int n_obst, Minerio minerios[], int usa_minerios) {
    int i;
    for (i = 0; i < n_obst; i++) {
        if (colide(x, y, w, h, obst[i].x, obst[i].y, obst[i].w, obst[i].h)) return 1;
    }
    if (usa_minerios) {
        for (i = 0; i < NUM_MINERIOS; i++) {
            if (minerios[i].ativo && colide(x, y, w, h, minerios[i].x, minerios[i].y, TAM_MINERIO, TAM_MINERIO)) return 1;
        }
    }
    return 0;
}

void add_obst(Retangulo lista[], int* n, float x, float y, float w, float h) {
    if (*n >= MAX_OBST) return;
    lista[*n].x = x;
    lista[*n].y = y;
    lista[*n].w = w;
    lista[*n].h = h;
    (*n)++;
}

void add_obj(Objeto lista[], int* n, ALLEGRO_BITMAP* img, float sx, float sy, float sw, float sh,
    float dx, float dy, float esc, float base_y) {
    if (*n >= MAX_OBJ) return;
    lista[*n].img = img;
    lista[*n].sx = sx;
    lista[*n].sy = sy;
    lista[*n].sw = sw;
    lista[*n].sh = sh;
    lista[*n].dx = dx;
    lista[*n].dy = dy;
    lista[*n].esc = esc;
    lista[*n].base_y = base_y;
    (*n)++;
}

// desenha os objetos que ficam ATRAS do jogador (depois = 0) ou NA FRENTE dele (depois = 1),
// comparando a linha do pe do objeto com a linha do pe do jogador
void desenhar_objetos(Objeto lista[], int n, float pes_y, int depois) {
    int i;
    for (i = 0; i < n; i++) {
        int atras = (lista[i].base_y <= pes_y);
        if ((!depois && atras) || (depois && !atras)) {
            al_draw_scaled_bitmap(lista[i].img, lista[i].sx, lista[i].sy, lista[i].sw, lista[i].sh,
                lista[i].dx, lista[i].dy, lista[i].sw * lista[i].esc, lista[i].sh * lista[i].esc, 0);
        }
    }
}

ALLEGRO_BITMAP* carregar(const char* arquivo) {
    ALLEGRO_BITMAP* b = al_load_bitmap(arquivo);
    if (!b) printf("ERRO: nao foi possivel carregar %s\n", arquivo);
    return b;
}

// ---------- JOGADOR E SLIMES (desenho) ----------

// o sprite (96x96) e maior que a hitbox (36x62), por isso o desenho sai um pouco para fora dela.
// Picareta usa outra folha, com quadros de 48x48.
void desenhar_jogador(Jogador* p) {
    int flip = 0;
    int linha = p->linha, coluna = p->coluna, frame;

    if (p->invulneravel % 10 >= 5) return;          // pisca quando leva dano
    if (p->dir == 3) flip = ALLEGRO_FLIP_HORIZONTAL; // as folhas olham para a direita

    if (p->tempo_ataque > 0) {
        frame = (p->tempo_ataque > 8) ? 0 : 1;

        if (p->ferramenta == 1) {
            int linha_acao = 0;                      // 0 = lado, 1 = baixo, 2 = cima
            if (p->dir == 0) linha_acao = 1;
            if (p->dir == 2) linha_acao = 2;
            al_draw_scaled_bitmap(img_player_acoes, frame * 48, linha_acao * 48, 48, 48,
                p->x - 54, p->y - 38, 144, 144, flip);
            return;
        }

        coluna = frame;
        if (p->dir == 1 || p->dir == 3) linha = 7;
        else if (p->dir == 2) linha = 8;
        else linha = 6;
    }

    al_draw_scaled_bitmap(img_player, coluna * 32, linha * 32, 32, 32, p->x - 30, p->y - 14, 96, 96, flip);
}

void desenhar_slime(Slime* s) {
    if (s->dano > 0) {
        al_draw_tinted_bitmap_region(img_slime, al_map_rgba(255, 50, 50, 255),
            s->frame * 64, 0, 64, 64, s->x - 16, s->y - 20, 0);
    }
    else {
        al_draw_bitmap_region(img_slime, s->frame * 64, 0, 64, 64, s->x - 16, s->y - 20, 0);
    }
    // barrinha de vida
    al_draw_filled_rectangle(s->x - 4, s->y - 14, s->x + 36, s->y - 9, al_map_rgb(100, 0, 0));
    al_draw_filled_rectangle(s->x - 4, s->y - 14, s->x - 4 + 40 * s->vida / 2, s->y - 9, al_map_rgb(0, 230, 0));
}

// ---------- MAPA (fase 1) ----------

void desenhar_regiao(ALLEGRO_BITMAP* bmp, float sx, float sy, float sw, float sh, float dx, float dy, float esc) {
    al_draw_scaled_bitmap(bmp, sx, sy, sw, sh, dx, dy, sw * esc, sh * esc, 0);
}

// preenche uma area com um tileset 3x3 (cantos, bordas e miolo), como o caminho e o lago
void desenhar_patch(ALLEGRO_BITMAP* tileset, float x, float y, int cols, int linhas) {
    int r, c;
    for (r = 0; r < linhas; r++) {
        for (c = 0; c < cols; c++) {
            int sx = (c == 0) ? 0 : (c == cols - 1 ? 2 : 1);
            int sy = (r == 0) ? 0 : (r == linhas - 1 ? 2 : 1);
            desenhar_regiao(tileset, sx * 16, sy * 16, 16, 16, x + c * TILE, y + r * TILE, 3);
        }
    }
}

// diz se uma celula do mapa esta livre para receber flores/tufos
// (longe do caminho, lago, casa, bau, poste e arvore do canto)
int celula_livre(int cx, int cy) {
    int px = cx * TILE, py = cy * TILE;
    if (py + TILE > CAMINHO_Y - 8 && py < CAMINHO_Y + 3 * TILE + 8) return 0;
    if (px + TILE > LAGO_X - TILE && px < LAGO_X + (LAGO_COLS + 1) * TILE &&
        py + TILE > LAGO_Y - TILE && py < LAGO_Y + (LAGO_LINHAS + 1) * TILE) return 0;
    if (px + TILE > 30 && px < 350 && py + TILE > 50 && py < 460) return 0;
    if (px + TILE > 470 && px < 600 && py + TILE > 260 && py < 360) return 0;
    if (px >= 840 && px < 930 && py >= 270 && py < 390) return 0;
    if (px > 1000 && py < 180) return 0;
    return 1;
}

void desenhar_chao_mapa() {
    int xx, yy, cx, cy;

    // grama
    for (yy = 0; yy < ALTURA; yy += TILE) {
        for (xx = 0; xx < LARGURA; xx += TILE) {
            desenhar_regiao(img_grama, 0, 0, 16, 16, (float)xx, (float)yy, 3);
        }
    }

    // tufos, flores e tulipas espalhados
    for (cy = 0; cy < ALTURA / TILE; cy++) {
        for (cx = 0; cx < LARGURA / TILE + 1; cx++) {
            int v, variacao;
            if (!celula_livre(cx, cy)) continue;
            v = (cx * 7 + cy * 13 + cx * cy) % 23;
            variacao = (cx + cy) % 3;
            if (v == 0) desenhar_regiao(img_decor, variacao * 16, 0, 16, 16, (float)(cx * TILE), (float)(cy * TILE), 3);
            if (v == 1) desenhar_regiao(img_decor, variacao * 16, 16, 16, 16, (float)(cx * TILE), (float)(cy * TILE), 3);
            if (v == 2) desenhar_regiao(img_decor, (variacao % 2) * 16, 128, 16, 16, (float)(cx * TILE), (float)(cy * TILE), 3);
        }
    }

    // caminho (comeca fora da tela para as pontas nao ficarem arredondadas)
    desenhar_patch(img_caminho, -TILE, CAMINHO_Y, LARGURA / TILE + 3, 3);

    // lago e ondinhas
    desenhar_patch(img_agua, LAGO_X, LAGO_Y, LAGO_COLS, LAGO_LINHAS);
    desenhar_regiao(img_agua, 0, 80, 16, 16, LAGO_X + 2 * TILE, LAGO_Y + 1 * TILE, 3);
    desenhar_regiao(img_agua, 16, 80, 16, 16, LAGO_X + 4 * TILE, LAGO_Y + 2 * TILE, 3);
    desenhar_regiao(img_agua, 32, 80, 16, 16, LAGO_X + 3 * TILE, LAGO_Y + 2 * TILE, 3);
}

// placa de madeira que marca a passagem entre mapa e caverna
void desenhar_placa(Retangulo r) {
    float cx = r.x + r.w / 2.0f;
    float pe = r.y + r.h / 2.0f + 24.0f;
    ALLEGRO_COLOR madeira = al_map_rgb(150, 100, 52);
    ALLEGRO_COLOR escura = al_map_rgb(86, 54, 26);
    ALLEGRO_COLOR claro = al_map_rgb(245, 235, 210);

    al_draw_filled_ellipse(cx, pe, 16, 5, al_map_rgba(0, 0, 0, 90));
    al_draw_filled_rectangle(cx - 3, pe - 28, cx + 3, pe, escura);
    al_draw_filled_rectangle(cx - 20, pe - 50, cx + 20, pe - 26, madeira);
    al_draw_rectangle(cx - 20, pe - 50, cx + 20, pe - 26, escura, 2.0f);
    al_draw_filled_rectangle(cx - 10, pe - 40, cx + 12, pe - 36, claro);
    al_draw_filled_triangle(cx - 16, pe - 38, cx - 8, pe - 45, cx - 8, pe - 31, claro);
}

// ---------- CAVERNA (fase 2) ----------

// chao e paredes de pedra (as paredes sao os 5 primeiros obstaculos da caverna)
void desenhar_fundo_caverna(Retangulo obst[]) {
    int k;
    al_clear_to_color(al_map_rgb(48, 42, 52));

    // pedrinhas no chao
    for (k = 0; k < 80; k++) {
        float px = (float)((k * 149) % LARGURA);
        float py = (float)(200 + (k * 83) % 430);
        al_draw_filled_rectangle(px, py, px + 6, py + 4, al_map_rgb(70, 64, 70));
    }

    // pocas d'agua
    al_draw_filled_ellipse(560, 560, 42, 14, al_map_rgb(30, 50, 70));
    al_draw_ellipse(560, 560, 42, 14, al_map_rgb(80, 110, 130), 1);
    al_draw_filled_ellipse(1000, 330, 36, 12, al_map_rgb(30, 50, 70));
    al_draw_ellipse(1000, 330, 36, 12, al_map_rgb(80, 110, 130), 1);
    al_draw_filled_ellipse(260, 430, 34, 11, al_map_rgb(30, 50, 70));
    al_draw_ellipse(260, 430, 34, 11, al_map_rgb(80, 110, 130), 1);

    // paredes
    for (k = 0; k < 5; k++) {
        al_draw_filled_rectangle(obst[k].x, obst[k].y, obst[k].x + obst[k].w, obst[k].y + obst[k].h, al_map_rgb(20, 18, 24));
    }
    al_draw_filled_rectangle(0, 194, LARGURA, 200, al_map_rgb(70, 64, 78));
    al_draw_filled_rectangle(0, 630, LARGURA, 636, al_map_rgb(70, 64, 78));

    // estalactites (descem do teto) e estalagmites (sobem do chao)
    for (k = 0; k < 32; k++) {
        float x = (float)(k * 40 + 20);
        float comp = (float)(15 + (k * 7) % 25);
        al_draw_filled_triangle(x - 8, 200, x + 8, 200, x, 200 + comp, al_map_rgb(60, 56, 68));
    }
    for (k = 0; k < 32; k++) {
        float x = (float)(k * 40 + 30);
        float comp = (float)(10 + (k * 5) % 20);
        al_draw_filled_triangle(x - 7, 630, x + 7, 630, x, 630 - comp, al_map_rgb(60, 56, 68));
    }
}

// adiciona uma decoracao da caverna (rocha, cristal...). cx = centro, base_y = onde pisa no chao.
// Se for solida, cria uma hitbox so na base dela.
void add_decor_cav(Objeto lista[], int* n, Retangulo obst[], int* n_obst, int img, float cx, float base_y, float esc, int solido) {
    float bw = (float)al_get_bitmap_width(img_cav[img]);
    float bh = (float)al_get_bitmap_height(img_cav[img]);
    float w = bw * esc, h = bh * esc;
    float x = cx - w / 2.0f;
    float y = base_y - h * 0.92f;

    if (solido) {
        add_obst(obst, n_obst, x + w * 0.25f, y + h * 0.62f, w * 0.5f, h * 0.30f);
        add_obj(lista, n, img_cav[img], 0, 0, bw, bh, x, y, esc, base_y);
    }
    else {
        add_obj(lista, n, img_cav[img], 0, 0, bw, bh, x, y, esc, y + h * 0.4f);   // fica sempre atras
    }
}

void resetar_caverna(Slime slimes[], Minerio minerios[], int* coletados) {
    float sx[NUM_SLIMES] = { 420, 760, 960, 980 };
    float sy[NUM_SLIMES] = { 300, 470, 270, 520 };
    float svel[NUM_SLIMES] = { 1.0f, 1.2f, 1.0f, 1.4f };
    float slim_e[NUM_SLIMES] = { 330, 620, 880, 900 };
    float slim_d[NUM_SLIMES] = { 620, 900, 1150, 1170 };
    float mx[NUM_MINERIOS] = { 300, 540, 650, 880, 1040, 340 };
    float my[NUM_MINERIOS] = { 430, 400, 340, 400, 360, 520 };
    int i;

    for (i = 0; i < NUM_SLIMES; i++) {
        slimes[i].x = sx[i];
        slimes[i].y = sy[i];
        slimes[i].vida = 2;
        slimes[i].dir = 1;
        slimes[i].vel = svel[i];
        slimes[i].lim_esq = slim_e[i];
        slimes[i].lim_dir = slim_d[i];
        slimes[i].frame = 0;
        slimes[i].contador = 0;
        slimes[i].dano = 0;
    }
    for (i = 0; i < NUM_MINERIOS; i++) {
        minerios[i].x = mx[i];
        minerios[i].y = my[i];
        minerios[i].vida = 3;
        minerios[i].ativo = 1;
    }
    *coletados = 0;
}

// ---------- INTERIOR DA CASA ----------

// cria um movel (escala 2). plano = fica sempre atras do jogador (tapete, janela); solido = bloqueia
void add_movel(Objeto lista[], int* n, Retangulo obst[], int* n_obst, ALLEGRO_BITMAP* img, float x, float y, int plano, int solido) {
    float bw = (float)al_get_bitmap_width(img);
    float bh = (float)al_get_bitmap_height(img);
    float w = bw * 2.0f, h = bh * 2.0f;

    add_obj(lista, n, img, 0, 0, bw, bh, x, y, 2.0f, plano ? -1000.0f : y + h);
    if (solido) {
        add_obst(obst, n_obst, x + 4, y + h * 0.5f, w - 8, h * 0.5f);   // so a metade de baixo bloqueia
    }
}

// piso e parede de fundo da sala (tiles de 16px desenhados em 32px)
void desenhar_interior_fundo() {
    int tile = 32, xx, yy;
    int sw = al_get_bitmap_width(img_int_piso);
    int sh = al_get_bitmap_height(img_int_piso);

    al_clear_to_color(al_map_rgb(15, 10, 8));

    for (yy = 120; yy < 600; yy += tile) {
        for (xx = 240; xx < 1040; xx += tile) {
            al_draw_scaled_bitmap(img_int_piso, 0, 0, sw, sh, (float)xx, (float)yy, (float)tile, (float)tile, 0);
        }
    }
    for (xx = 240; xx < 1040; xx += tile) {
        al_draw_scaled_bitmap(img_int_parede_topo, 0, 0, sw, sh, (float)xx, 56, (float)tile, (float)tile, 0);
        al_draw_scaled_bitmap(img_int_parede_base, 0, 0, sw, sh, (float)xx, 56 + (float)tile, (float)tile, (float)tile, 0);
    }
    al_draw_rectangle(240, 56, 1040, 600, al_map_rgb(30, 20, 10), 4);
}

// menu bem simples: ENTER joga, ESC sai
int menu(ALLEGRO_DISPLAY* janela) {
    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();
    ALLEGRO_EVENT ev;
    int resultado = -1;

    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_display_event_source(janela));

    while (resultado == -1) {
        al_clear_to_color(al_map_rgb(20, 30, 80));
        desenhar_texto("ELEMENTAL ADVENTURE", LARGURA / 2, 200, 6, al_map_rgb(255, 220, 80), 1);
        desenhar_texto("Jogo educativo de Quimica", LARGURA / 2, 270, 3, al_map_rgb(255, 255, 255), 1);
        desenhar_texto("ENTER - Jogar", LARGURA / 2, 400, 4, al_map_rgb(255, 255, 255), 1);
        desenhar_texto("ESC - Sair", LARGURA / 2, 450, 4, al_map_rgb(255, 255, 255), 1);
        al_flip_display();

        al_wait_for_event(fila, &ev);
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) resultado = 0;
        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ENTER) resultado = 1;
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) resultado = 0;
        }
    }
    al_destroy_event_queue(fila);
    return resultado;
}

int main() {
    if (!al_init()) {
        printf("Erro ao iniciar o Allegro\n");
        return -1;
    }
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_image_addon();

    ALLEGRO_DISPLAY* janela = al_create_display(LARGURA, ALTURA);
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();
    ALLEGRO_EVENT ev;
    int i;

    if (!janela || !timer || !fila) {
        printf("Erro ao criar janela ou timer\n");
        return -1;
    }

    // ---------- carregar imagens ----------
    static const char* arq_cav[CAV_NUM_IMGS] = {
        CAV_PASTA "128/Beige_rpck_dark_shadow1.png",
        CAV_PASTA "64/Beige_rpck_dark_shadow2.png",
        CAV_PASTA "64/Beige_rpck_dark_shadow3.png",
        CAV_PASTA "64/Blue-green_crystal_dark_shadow1.png",
        CAV_PASTA "32/Blue-green_crystal_dark_shadow3.png",
        CAV_PASTA "64/mushroom1_dark_shadow1.png",
        CAV_PASTA "64/Bonefire_dark_shadow.png",
        CAV_PASTA "32/Human_skeleton_dark_shadow.png"
    };
    int imagens_ok = 1;

    img_player = carregar("Assets/Player.png");
    img_player_acoes = carregar("Assets/Player_Actions.png");
    img_slime = carregar("Assets/Slime_Green.png");
    img_grama = carregar("Assets/Grass_Middle.png");
    img_caminho = carregar("Assets/Path_Tile.png");
    img_agua = carregar("Assets/Water_Tile.png");
    img_decor = carregar("Assets/Outdoor_Decor_Free.png");
    img_casa = carregar("Assets/House_1_Wood_Base_Blue.png");
    img_arvore = carregar("Assets/Oak_Tree.png");
    img_arvore_peq = carregar("Assets/Oak_Tree_Small.png");
    img_bau = carregar("Assets/Chest.png");
    img_int_piso = carregar("Assets/Interior_Floor.png");
    img_int_parede_topo = carregar("Assets/Interior_Wall_Top.png");
    img_int_parede_base = carregar("Assets/Interior_Wall_Base.png");
    img_int_capacho = carregar("Assets/Interior_Door_Mat.png");
    img_int_cama = carregar("Assets/Interior_Bed.png");
    img_int_mesa = carregar("Assets/Interior_Table.png");
    img_int_escrivaninha = carregar("Assets/Interior_Desk.png");
    img_int_tapete = carregar("Assets/Interior_Rug.png");
    img_int_fornalha = carregar("Assets/Interior_Furnace.png");
    img_int_estante = carregar("Assets/Interior_Bookshelf.png");
    img_int_janela = carregar("Assets/Interior_Window.png");
    img_int_bau = carregar("Assets/Interior_Chest.png");
    for (i = 0; i < CAV_NUM_IMGS; i++) {
        img_cav[i] = carregar(arq_cav[i]);
        if (!img_cav[i]) imagens_ok = 0;
    }

    if (!img_player || !img_player_acoes || !img_slime || !img_grama || !img_caminho || !img_agua ||
        !img_decor || !img_casa || !img_arvore || !img_arvore_peq || !img_bau ||
        !img_int_piso || !img_int_parede_topo || !img_int_parede_base || !img_int_capacho ||
        !img_int_cama || !img_int_mesa || !img_int_escrivaninha || !img_int_tapete ||
        !img_int_fornalha || !img_int_estante || !img_int_janela || !img_int_bau || !imagens_ok) {
        printf("Verifique se as pastas Assets e assets_cave estao na pasta do projeto.\n");
        return -1;
    }

    if (menu(janela) == 0) return 0;

    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));

    // ---------- jogador ----------
    //             x    y   w   h  vel vida     dir inv atq ferr col lin anim
    Jogador p = { 600, 450, 36, 62, 5, VIDA_MAX, 0, 0, 0, 0, 0, 0, 0 };

    // cena: 0 = mapa, 1 = caverna, 2 = casa
    int cena = 0;

    // ---------- listas de obstaculos e de objetos de cada cena ----------
    Retangulo obst_mapa[MAX_OBST], obst_cav[MAX_OBST], obst_casa[MAX_OBST];
    Objeto obj_mapa[MAX_OBJ], obj_cav[MAX_OBJ], obj_casa[MAX_OBJ];
    int n_obst_mapa = 0, n_obst_cav = 0, n_obst_casa = 0;
    int n_obj_mapa = 0, n_obj_cav = 0, n_obj_casa = 0;

    // ----- mapa: casa -----
    // medidas da casa (ja ajustadas no jogo completo)
    float casa_x = 40, casa_y = 60;
    float casa_h = al_get_bitmap_height(img_casa) * 3.0f;
    float base_casa = casa_y + casa_h - 10;
    Retangulo parede_casa = { casa_x + 40, base_casa - 369, 210, 335 };
    float linha_chao = parede_casa.y + parede_casa.h;
    Retangulo porta_casa = { casa_x + 100, linha_chao - 26, 48, 14 };

    // as paredes bloqueiam, mas deixam o vao da porta aberto
    add_obst(obst_mapa, &n_obst_mapa, parede_casa.x, parede_casa.y, porta_casa.x - parede_casa.x, parede_casa.h);
    add_obst(obst_mapa, &n_obst_mapa, porta_casa.x + porta_casa.w, parede_casa.y,
        (parede_casa.x + parede_casa.w) - (porta_casa.x + porta_casa.w), parede_casa.h);
    add_obj(obj_mapa, &n_obj_mapa, img_casa, 0, 0, (float)al_get_bitmap_width(img_casa), (float)al_get_bitmap_height(img_casa),
        casa_x, casa_y, 3, porta_casa.y);

    // ----- mapa: lago -----
    add_obst(obst_mapa, &n_obst_mapa, LAGO_X + 18, LAGO_Y + 20, LAGO_COLS * TILE - 38, LAGO_LINHAS * TILE - 42);

    // ----- mapa: arvores grandes (so o tronco bloqueia) -----
    float arv[4][2] = { { 0, 0 },{ 1100, 20 },{ 1140, 540 },{ 20, 560 } };
    for (i = 0; i < 4; i++) {
        add_obst(obst_mapa, &n_obst_mapa, arv[i][0] + 52, arv[i][1] + 108, 24, 20);
        add_obj(obj_mapa, &n_obj_mapa, img_arvore, 0, 0, (float)al_get_bitmap_width(img_arvore), (float)al_get_bitmap_height(img_arvore),
            arv[i][0], arv[i][1], 2, arv[i][1] + 128);
    }

    // ----- mapa: arvores pequenas (a folha tem 2 arvores, alterna entre elas) -----
    float arv_p[3][2] = { { 300, 100 },{ 500, 150 },{ 1000, 560 } };
    for (i = 0; i < 3; i++) {
        float base = (i % 2) ? 87.0f : 96.0f;
        add_obst(obst_mapa, &n_obst_mapa, arv_p[i][0] + 33, arv_p[i][1] + base - 18, 18, 18);
        add_obj(obj_mapa, &n_obj_mapa, img_arvore_peq, (float)(34 + (i % 2) * 32), 0, 28, 48,
            arv_p[i][0], arv_p[i][1], 3, arv_p[i][1] + base);
    }

    // ----- mapa: postes de luz e bau -----
    add_obj(obj_mapa, &n_obj_mapa, img_decor, 68, 72, 10, 40, POSTE_ESQ_X, POSTE_Y, 3, POSTE_Y + 117);
    add_obj(obj_mapa, &n_obj_mapa, img_decor, 68, 72, 10, 40, POSTE_DIR_X, POSTE_Y, 3, POSTE_Y + 117);
    add_obst(obst_mapa, &n_obst_mapa, POSTE_ESQ_X, POSTE_Y + 96, 27, 21);
    add_obst(obst_mapa, &n_obst_mapa, POSTE_DIR_X, POSTE_Y + 96, 27, 21);
    add_obj(obj_mapa, &n_obj_mapa, img_bau, 0, 0, 16, 16, BAU_X, BAU_Y, 2, BAU_Y + 32);
    add_obst(obst_mapa, &n_obst_mapa, BAU_X, BAU_Y + 12, 32, 20);

    // ----- caverna: paredes (as 5 primeiras) e decoracoes -----
    add_obst(obst_cav, &n_obst_cav, 0, 0, LARGURA, 200);          // teto
    add_obst(obst_cav, &n_obst_cav, 0, 630, LARGURA, 90);         // chao de baixo
    add_obst(obst_cav, &n_obst_cav, 0, 200, 40, 100);             // esquerda (acima da saida)
    add_obst(obst_cav, &n_obst_cav, 0, 480, 40, 150);             // esquerda (abaixo da saida)
    add_obst(obst_cav, &n_obst_cav, 1240, 200, 40, 430);          // direita
    add_decor_cav(obj_cav, &n_obj_cav, obst_cav, &n_obst_cav, CAV_ROCHA_M, 330, 262, 1.5f, 1);
    add_decor_cav(obj_cav, &n_obj_cav, obst_cav, &n_obst_cav, CAV_CRISTAL_M, 600, 255, 1.5f, 1);
    add_decor_cav(obj_cav, &n_obj_cav, obst_cav, &n_obst_cav, CAV_ROCHA_G, 930, 268, 1.0f, 1);
    add_decor_cav(obj_cav, &n_obj_cav, obst_cav, &n_obst_cav, CAV_COGUMELO, 1150, 275, 1.5f, 1);
    add_decor_cav(obj_cav, &n_obj_cav, obst_cav, &n_obst_cav, CAV_CRISTAL_P, 240, 585, 2.0f, 1);
    add_decor_cav(obj_cav, &n_obj_cav, obst_cav, &n_obst_cav, CAV_ROCHA_P, 560, 590, 1.5f, 1);
    add_decor_cav(obj_cav, &n_obj_cav, obst_cav, &n_obst_cav, CAV_COGUMELO, 800, 585, 1.5f, 1);
    add_decor_cav(obj_cav, &n_obj_cav, obst_cav, &n_obst_cav, CAV_CRISTAL_M, 1060, 580, 1.5f, 1);
    add_decor_cav(obj_cav, &n_obj_cav, obst_cav, &n_obst_cav, CAV_ESQUELETO, 420, 440, 2.0f, 0);
    add_decor_cav(obj_cav, &n_obj_cav, obst_cav, &n_obst_cav, CAV_FOGUEIRA, 220, 300, 1.5f, 0);

    // ----- casa por dentro: paredes e moveis (x, y em pixels de tela) -----
    add_obst(obst_casa, &n_obst_casa, 0, 0, 240, ALTURA);
    add_obst(obst_casa, &n_obst_casa, 1040, 0, 240, ALTURA);
    add_obst(obst_casa, &n_obst_casa, 0, 0, LARGURA, 120);
    add_obst(obst_casa, &n_obst_casa, 0, 600, LARGURA, 120);
    add_movel(obj_casa, &n_obj_casa, obst_casa, &n_obst_casa, img_int_tapete, 590, 330, 1, 0);
    add_movel(obj_casa, &n_obj_casa, obst_casa, &n_obst_casa, img_int_capacho, 608, 568, 1, 0);
    add_movel(obj_casa, &n_obj_casa, obst_casa, &n_obst_casa, img_int_janela, 560, 62, 1, 0);
    add_movel(obj_casa, &n_obj_casa, obst_casa, &n_obst_casa, img_int_janela, 860, 62, 1, 0);
    add_movel(obj_casa, &n_obj_casa, obst_casa, &n_obst_casa, img_int_cama, 264, 106, 0, 1);
    add_movel(obj_casa, &n_obj_casa, obst_casa, &n_obst_casa, img_int_estante, 400, 90, 0, 1);
    add_movel(obj_casa, &n_obj_casa, obst_casa, &n_obst_casa, img_int_fornalha, 930, 60, 0, 1);
    add_movel(obj_casa, &n_obj_casa, obst_casa, &n_obst_casa, img_int_escrivaninha, 280, 330, 0, 1);
    add_movel(obj_casa, &n_obj_casa, obst_casa, &n_obst_casa, img_int_mesa, 612, 340, 0, 1);
    add_movel(obj_casa, &n_obj_casa, obst_casa, &n_obst_casa, img_int_bau, 975, 330, 0, 1);

    // ---------- portas ----------
    Retangulo portal = { 0, CAMINHO_Y, 40, 3 * TILE };   // entrada da caverna (ponta esquerda do caminho)
    Retangulo saida = { 0, 300, 70, 180 };               // saida da caverna
    Retangulo porta_int = { 608, 572, 64, 28 };          // capacho (saida da casa)

    Slime slimes[NUM_SLIMES];
    Minerio minerios[NUM_MINERIOS];
    int coletados = 0;
    int sair = 0;
    char texto[50];

    resetar_caverna(slimes, minerios, &coletados);
    al_start_timer(timer);

    while (!sair) {
        al_wait_for_event(fila, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) sair = 1;

        // ataques (uma vez por aperto de tecla)
        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) sair = 1;

            if ((ev.keyboard.keycode == ALLEGRO_KEY_E || ev.keyboard.keycode == ALLEGRO_KEY_Q) && p.tempo_ataque == 0) {
                Retangulo g = area_golpe(&p);
                p.tempo_ataque = 16;
                p.ferramenta = (ev.keyboard.keycode == ALLEGRO_KEY_Q) ? 1 : 0;

                if (cena == 1) {
                    if (p.ferramenta == 0) {
                        // espada: acerta slime
                        for (i = 0; i < NUM_SLIMES; i++) {
                            if (slimes[i].vida > 0 && colide(g.x, g.y, g.w, g.h, slimes[i].x, slimes[i].y, SLIME_W, SLIME_H)) {
                                slimes[i].vida--;
                                slimes[i].dano = 12;
                            }
                        }
                    }
                    else {
                        // picareta: acerta minerio
                        for (i = 0; i < NUM_MINERIOS; i++) {
                            if (minerios[i].ativo && colide(g.x, g.y, g.w, g.h, minerios[i].x, minerios[i].y, TAM_MINERIO, TAM_MINERIO)) {
                                minerios[i].vida--;
                                if (minerios[i].vida <= 0) {
                                    minerios[i].ativo = 0;
                                    coletados++;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            // ---------- UPDATE ----------
            ALLEGRO_KEYBOARD_STATE teclado;
            al_get_keyboard_state(&teclado);

            float nx = p.x, ny = p.y;
            int andando = 0;
            if (p.tempo_ataque == 0) {
                if (al_key_down(&teclado, ALLEGRO_KEY_UP)) { ny -= p.vel; p.dir = 2; p.linha = 5; andando = 1; }
                else if (al_key_down(&teclado, ALLEGRO_KEY_DOWN)) { ny += p.vel; p.dir = 0; p.linha = 3; andando = 1; }
                else if (al_key_down(&teclado, ALLEGRO_KEY_RIGHT)) { nx += p.vel; p.dir = 1; p.linha = 4; andando = 1; }
                else if (al_key_down(&teclado, ALLEGRO_KEY_LEFT)) { nx -= p.vel; p.dir = 3; p.linha = 4; andando = 1; }
            }

            // limites da tela
            if (nx < 0) nx = 0;
            if (nx + p.w > LARGURA) nx = (float)(LARGURA - p.w);
            if (ny < 0) ny = 0;
            if (ny + p.h > ALTURA) ny = (float)(ALTURA - p.h);

            // colisao com os obstaculos da cena atual (testa x e y separados, so com os pes)
            Retangulo* obst = obst_mapa;
            int n_obst = n_obst_mapa;
            int usa_minerios = 0;
            if (cena == 1) { obst = obst_cav;  n_obst = n_obst_cav;  usa_minerios = 1; }
            if (cena == 2) { obst = obst_casa; n_obst = n_obst_casa; }

            if (!bateu(nx, p.y + p.h - 10, p.w, 10, obst, n_obst, minerios, usa_minerios)) p.x = nx;
            if (!bateu(p.x, ny + p.h - 10, p.w, 10, obst, n_obst, minerios, usa_minerios)) p.y = ny;

            // animacao de caminhada
            if (andando) {
                p.contador_anim++;
                if (p.contador_anim >= 8) {
                    p.coluna = (p.coluna + 1) % 6;
                    p.contador_anim = 0;
                }
            }

            // trocar de cena pisando nas portas
            if (cena == 0) {
                if (colide_pes(&p, portal)) {
                    cena = 1;
                    p.x = 100;
                    p.y = 330;
                    p.dir = 1;
                }
                else if (colide_pes(&p, porta_casa)) {
                    cena = 2;
                    p.x = 640 - p.w / 2;
                    p.y = 470;
                    p.dir = 2;
                }
            }
            else if (cena == 1) {
                if (colide_pes(&p, saida)) {
                    cena = 0;
                    p.x = 60;
                    p.y = CAMINHO_Y + 40;
                    p.dir = 1;
                }
            }
            else {
                if (colide_pes(&p, porta_int)) {
                    cena = 0;
                    p.x = porta_casa.x + porta_casa.w / 2 - p.w / 2;
                    p.y = linha_chao - 48;
                    p.dir = 0;
                }
            }

            // slimes (so na caverna)
            if (cena == 1) {
                for (i = 0; i < NUM_SLIMES; i++) {
                    if (slimes[i].vida <= 0) continue;

                    slimes[i].x += slimes[i].vel * slimes[i].dir;
                    if (slimes[i].x < slimes[i].lim_esq) slimes[i].dir = 1;
                    if (slimes[i].x > slimes[i].lim_dir) slimes[i].dir = -1;
                    // se bater numa pedra, volta
                    if (bateu(slimes[i].x, slimes[i].y, SLIME_W, SLIME_H, obst_cav, n_obst_cav, minerios, 1)) {
                        slimes[i].dir = -slimes[i].dir;
                        slimes[i].x += slimes[i].vel * slimes[i].dir * 2;
                    }

                    // encostou no jogador
                    if (p.invulneravel == 0 && colide(p.x, p.y, p.w, p.h, slimes[i].x, slimes[i].y, SLIME_W, SLIME_H)) {
                        p.vida--;
                        p.invulneravel = 60;
                    }

                    // animacao do slime
                    slimes[i].contador++;
                    if (slimes[i].contador >= 10) {
                        slimes[i].frame = (slimes[i].frame + 1) % 4;
                        slimes[i].contador = 0;
                    }
                    if (slimes[i].dano > 0) slimes[i].dano--;
                }
            }

            if (p.invulneravel > 0) p.invulneravel--;
            if (p.tempo_ataque > 0) p.tempo_ataque--;

            // morreu: volta pro mapa e a caverna recomeca
            if (p.vida <= 0) {
                p.vida = VIDA_MAX;
                p.x = 600;
                p.y = 450;
                cena = 0;
                resetar_caverna(slimes, minerios, &coletados);
            }

            // ---------- DESENHO ----------
            float pes_y = p.y + p.h;

            // 1) o que fica atras do jogador
            if (cena == 0) {
                desenhar_chao_mapa();
                desenhar_placa(portal);
                desenhar_objetos(obj_mapa, n_obj_mapa, pes_y, 0);
            }
            else if (cena == 1) {
                desenhar_fundo_caverna(obst_cav);
                desenhar_placa(saida);
                desenhar_objetos(obj_cav, n_obj_cav, pes_y, 0);

                for (i = 0; i < NUM_MINERIOS; i++) {
                    if (minerios[i].ativo) {
                        float mx = minerios[i].x, my = minerios[i].y;
                        al_draw_filled_ellipse(mx + TAM_MINERIO / 2, my + TAM_MINERIO - 4, 26, 7, al_map_rgba(0, 0, 0, 90));
                        al_draw_filled_rounded_rectangle(mx, my, mx + TAM_MINERIO, my + TAM_MINERIO - 6, 12, 12, al_map_rgb(104, 104, 116));
                        al_draw_filled_rectangle(mx + 8, my + 8, mx + 24, my + 20, al_map_rgb(184, 90, 42));
                        al_draw_filled_rectangle(mx + 30, my + 24, mx + 46, my + 38, al_map_rgb(238, 148, 76));
                    }
                }
                for (i = 0; i < NUM_SLIMES; i++) {
                    if (slimes[i].vida > 0) desenhar_slime(&slimes[i]);
                }
            }
            else {
                desenhar_interior_fundo();
                desenhar_objetos(obj_casa, n_obj_casa, pes_y, 0);
            }

            // 2) jogador
            desenhar_jogador(&p);

            // 3) o que fica na frente do jogador
            if (cena == 0) desenhar_objetos(obj_mapa, n_obj_mapa, pes_y, 1);
            else if (cena == 1) desenhar_objetos(obj_cav, n_obj_cav, pes_y, 1);
            else desenhar_objetos(obj_casa, n_obj_casa, pes_y, 1);

            // HUD: vida e ferro
            al_draw_filled_rectangle(10, 10, 10 + 20 * VIDA_MAX, 26, al_map_rgb(100, 0, 0));
            al_draw_filled_rectangle(10, 10, 10 + 20 * p.vida, 26, al_map_rgb(0, 220, 0));
            snprintf(texto, sizeof(texto), "Ferro (Fe): %d/%d", coletados, NUM_MINERIOS);
            desenhar_texto(texto, 10, 35, 3, al_map_rgb(255, 255, 255), 0);
            if (coletados == NUM_MINERIOS) {
                desenhar_texto("FASE COMPLETA!", LARGURA / 2, 20, 4, al_map_rgb(255, 255, 0), 1);
            }

            al_flip_display();
        }
    }

    // liberar imagens
    al_destroy_bitmap(img_player);
    al_destroy_bitmap(img_player_acoes);
    al_destroy_bitmap(img_slime);
    al_destroy_bitmap(img_grama);
    al_destroy_bitmap(img_caminho);
    al_destroy_bitmap(img_agua);
    al_destroy_bitmap(img_decor);
    al_destroy_bitmap(img_casa);
    al_destroy_bitmap(img_arvore);
    al_destroy_bitmap(img_arvore_peq);
    al_destroy_bitmap(img_bau);
    al_destroy_bitmap(img_int_piso);
    al_destroy_bitmap(img_int_parede_topo);
    al_destroy_bitmap(img_int_parede_base);
    al_destroy_bitmap(img_int_capacho);
    al_destroy_bitmap(img_int_cama);
    al_destroy_bitmap(img_int_mesa);
    al_destroy_bitmap(img_int_escrivaninha);
    al_destroy_bitmap(img_int_tapete);
    al_destroy_bitmap(img_int_fornalha);
    al_destroy_bitmap(img_int_estante);
    al_destroy_bitmap(img_int_janela);
    al_destroy_bitmap(img_int_bau);
    for (i = 0; i < CAV_NUM_IMGS; i++) al_destroy_bitmap(img_cav[i]);

    al_destroy_timer(timer);
    al_destroy_event_queue(fila);
    al_destroy_display(janela);
    return 0;
}