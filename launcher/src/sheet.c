/* NumPlay spreadsheet: 10 columns x 30 rows, deliberately self-contained. */
#include "ui.h"
#include "reader_docs.h"
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>

#define COLS 10
#define ROWS 30
#define CELL_MAX 64
#define VIS_COLS 8
#define VIS_ROWS 9

/* Couleurs en RGB565 (16 bits) */
#define RGB565(r,g,b) ((color_t)((((r)>>3)<<11)|(((g)>>2)<<5)|((b)>>3)))
#define NAVY  RGB565(0x10,0x19,0x23)
#define BLUE  RGB565(0x24,0x36,0x4A)
#define TEAL  RGB565(0x41,0xC7,0xA1)
#define GOLD  RGB565(0xF4,0xC9,0x5D)
#define WHITE ((color_t)0xFFFF)

static char cells[ROWS][COLS][CELL_MAX];
static float values[ROWS][COLS];
static unsigned char state[ROWS][COLS]; /* 0=a calculer, 1=en cours, 2=fait */
static int cur_r, cur_c, row0, col0, editing, menu;
static char edit[CELL_MAX];
static ui_keys_t keys;

/* ---------- Analyse des formules ---------- */

static int colnum(const char **p) {
  int c = 0;
  while (isalpha((unsigned char)**p)) {
    c = c * 26 + (toupper((unsigned char)**p) - 'A' + 1);
    (*p)++;
  }
  return c - 1;
}

static int readnum(const char **p) {
  int n = 0;
  while (isdigit((unsigned char)**p)) {
    n = n * 10 + (**p - '0');
    (*p)++;
  }
  return n;
}

static void skip(const char **p) {
  while (**p == ' ' || **p == '\t') (*p)++;
}

static float expr(const char **p, int *err);

static float cellval(int r, int c, int *err) {
  if (r < 0 || r >= ROWS || c < 0 || c >= COLS) { *err = 1; return 0; }
  if (state[r][c] == 1) { *err = 1; return 0; }      /* reference circulaire */
  if (state[r][c] == 2) return values[r][c];
  state[r][c] = 1;
  const char *s = cells[r][c];
  char *e;
  if (!*s) {
    values[r][c] = 0;
  } else if (*s != '=') {
    values[r][c] = strtof(s, &e);
    if (e == s) *err = 1;
  } else {
    const char *p = s + 1;
    values[r][c] = expr(&p, err);
    skip(&p);
    if (*p) *err = 1;
  }
  state[r][c] = 2;
  return values[r][c];
}

/* fn: 1=SOMME 2=MIN 3=MAX 4=MOYENNE */
static float range(const char **p, int *err, int fn) {
  skip(p);
  int c1 = colnum(p);
  int r1 = readnum(p) - 1;
  skip(p);
  int c2 = c1, r2 = r1;
  if (**p == ':') {
    (*p)++;
    skip(p);
    c2 = colnum(p);
    r2 = readnum(p) - 1;
  }
  if (c1 < 0 || c2 < 0 || r1 < 0 || r2 < 0) { *err = 1; return 0; }

  float out = (fn == 1 || fn == 4) ? 0.f : (fn == 2 ? 999999.f : -999999.f);
  int n = 0;
  for (int r = NP_MIN(r1, r2); r <= NP_MAX(r1, r2); r++) {
    for (int c = NP_MIN(c1, c2); c <= NP_MAX(c1, c2); c++) {
      float v = cellval(r, c, err);
      if (fn == 1 || fn == 4) out += v;
      else if (fn == 2 && v < out) out = v;
      else if (fn == 3 && v > out) out = v;
      n++;
    }
  }
  return fn == 4 ? (n ? out / n : 0.f) : out;
}

static float factor(const char **p, int *err) {
  skip(p);
  if (**p == '(') {
    (*p)++;
    float v = expr(p, err);
    skip(p);
    if (**p == ')') (*p)++; else *err = 1;
    return v;
  }
  if (**p == '-') { (*p)++; return -factor(p, err); }
  if (isalpha((unsigned char)**p)) {
    const char *q = *p;
    int c = colnum(&q);
    int r = readnum(&q);
    if (c >= 0 && r > 0) { *p = q; return cellval(r - 1, c, err); }
    *err = 1;
    return 0;
  }
  char *e;
  float v = strtof(*p, &e);
  if (e == *p) *err = 1;
  *p = e;
  return v;
}

static float term(const char **p, int *err) {
  float v = factor(p, err);
  for (;;) {
    skip(p);
    char op = **p;
    if (op != '*' && op != '/') return v;
    (*p)++;
    float b = factor(p, err);
    if (op == '*') v *= b;
    else if (b == 0) *err = 1;
    else v /= b;
  }
}

static int func(const char **p, const char *name, int len, int fn, float *v, int *err) {
  if (strncasecmp(*p, name, len)) return 0;
  *p += len;
  *v = range(p, err, fn);
  if (**p == ')') (*p)++; else *err = 1;
  return 1;
}

static float expr(const char **p, int *err) {
  skip(p);
  float v;
  if (func(p, "SOMME(", 6, 1, &v, err)) return v;
  if (func(p, "MOYENNE(", 8, 4, &v, err)) return v;
  if (func(p, "MIN(", 4, 2, &v, err)) return v;
  if (func(p, "MAX(", 4, 3, &v, err)) return v;
  v = term(p, err);
  for (;;) {
    skip(p);
    char op = **p;
    if (op != '+' && op != '-') return v;
    (*p)++;
    float b = term(p, err);
    v = (op == '+') ? v + b : v - b;
  }
}

static void recalc(void) {
  memset(state, 0, sizeof state);
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      int e = 0;
      cellval(r, c, &e);
      if (e) values[r][c] = NAN;
    }
  }
}

/* ---------- Saisie ---------- */

static void putkey(char ch) {
  size_t n = strlen(edit);
  if (n + 1 < CELL_MAX) { edit[n] = ch; edit[n + 1] = 0; }
}

static void putstr(const char *s) {
  while (*s) putkey(*s++);
}

/* Menu PI : chaque appui insere le jeton suivant dans la barre de saisie. */
static void menu_key(void) {
  static const char *const token[] = {
    "=", "SOMME(", "MOYENNE(", "MIN(", "MAX(",
    "+", "-", "*", "/", "(", ")", ":",
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J"
  };
  const int n = (int)(sizeof token / sizeof token[0]);
  putstr(token[menu % n]);
  menu = (menu + 1) % n;
}

/* ---------- Affichage ---------- */

static void header(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, BLUE, NAVY);
  gfx_text(&np_font_title, 12, 25, "NumPlay", WHITE, 256);
  gfx_text(&np_font_small, 112, 24, "TABLEUR  A-J / 1-30", WHITE, 220);
  gfx_rrect(10, 31, 300, 24, 4, WHITE, 245);
  gfx_text(&np_font_small, 16, 48, edit, BLUE, 256);
}

static void scene(void *x) {
  (void)x;
  header();
  int x0 = 10, y0 = 61, cw = 34, rh = 17;
  gfx_rrect(x0, y0, 300, 150, 2, WHITE, 245);

  for (int c = 0; c < VIS_COLS && col0 + c < COLS; c++) {
    char s[2] = { (char)('A' + col0 + c), 0 };
    gfx_text_center(&np_font_small, x0 + 22 + c * cw, y0 + 13, s, BLUE, 256);
  }

  for (int r = 0; r < VIS_ROWS && row0 + r < ROWS; r++) {
    char s[4];
    snprintf(s, sizeof s, "%d", row0 + r + 1);
    gfx_text_right(&np_font_small, x0 + 18, y0 + 29 + r * rh, s, BLUE, 220);

    for (int c = 0; c < VIS_COLS && col0 + c < COLS; c++) {
      int cc = col0 + c, rr = row0 + r;
      int xx = x0 + 22 + c * cw;
      int sel = (rr == cur_r && cc == cur_c);
      if (sel) gfx_fill(xx, y0 + 17 + r * rh, cw, rh, TEAL);

      char out[16];
      if (cells[rr][cc][0] == '=') {
        if (isnan(values[rr][cc])) strcpy(out, "#ERR");
        else snprintf(out, sizeof out, "%.5g", (double)values[rr][cc]);
      } else {
        snprintf(out, sizeof out, "%.12s", cells[rr][cc]);
      }
      gfx_text_center(&np_font_small, xx + cw / 2, y0 + 29 + r * rh, out,
                      sel ? WHITE : BLUE, 256);
    }
  }

  gfx_text(&np_font_small, 10, 225,
           editing ? "Fonctions: =  SOMME(  MOYENNE(  MIN(  MAX(  A1:B3"
                   : "Fleches: naviguer  OK/EXE: editer/valider  DEL: effacer  PI: fonctions",
           WHITE, 210);
}

/* ---------- Boucle principale ---------- */

int np_sheet(void) {
  memset(cells, 0, sizeof cells);
  cur_r = cur_c = row0 = col0 = editing = menu = 0;
  edit[0] = 0;
  np_wait_release();
  keys = (ui_keys_t){0, 0, 0};

  /* Code secret "2011" : suite des 4 derniers chiffres tapes */
  char sec[4] = {0, 0, 0, 0};

  for (;;) {
    recalc();
    ui_frame(scene, 0);
    uint32_t p = ui_poll(&keys);
    if (p & K_HOME) return -2;

    if (!editing) {
      if (p & K_LEFT)  cur_c = NP_MAX(0, cur_c - 1);
      if (p & K_RIGHT) cur_c = NP_MIN(COLS - 1, cur_c + 1);
      if (p & K_UP)    cur_r = NP_MAX(0, cur_r - 1);
      if (p & K_DOWN)  cur_r = NP_MIN(ROWS - 1, cur_r + 1);
      if (cur_r < row0) row0 = cur_r;
      if (cur_r >= row0 + VIS_ROWS) row0 = cur_r - (VIS_ROWS - 1);
      if (cur_c < col0) col0 = cur_c;
      if (cur_c >= col0 + VIS_COLS) col0 = cur_c - (VIS_COLS - 1);
      if (p & K_BACKSPACE) cells[cur_r][cur_c][0] = 0;
      if (p & K_OK) {
        editing = 1;
        strcpy(edit, cells[cur_r][cur_c]);
      }
    } else {
      if (p & K_BACK) { editing = 0; edit[0] = 0; }
      if (p & K_BACKSPACE) {
        size_t n = strlen(edit);
        if (n) edit[n - 1] = 0;
      }
      if (p & K_PI) menu_key();
      if (p & K_OK) {
        strncpy(cells[cur_r][cur_c], edit, CELL_MAX - 1);
        cells[cur_r][cur_c][CELL_MAX - 1] = 0;
        editing = 0;
        edit[0] = 0;
      }
      for (int d = 0; d < 10; d++)
        if (p & (K_DIGIT0 << d)) putkey((char)('0' + d));
    }

    /* Detection du code secret 2-0-1-1 (hors edition ou pendant) */
    for (int d = 0; d < 10; d++) {
      if (p & (K_DIGIT0 << d)) {
        memmove(sec, sec + 1, 3);
        sec[3] = (char)('0' + d);
        if (!memcmp(sec, "2011", 4)) {
          memset(sec, 0, sizeof sec);
          edit[0] = 0;
          editing = 0;
          np_home(0, true);
          np_wait_release();
        }
      }
    }

    np_sleep(20);
  }
}
