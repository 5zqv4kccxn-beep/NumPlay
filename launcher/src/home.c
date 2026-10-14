/* NumPlay reader: fake spreadsheet entry and embedded study documents. */
#include <eadk.h>
#include "ui.h"
#include "reader_docs.h"
#include "sys.h"

#define DOCS_TOP 0x24364A
#define DOCS_BOTTOM 0x101923
#define ACCENT 0x41C7A1
#define TABLE_TOP 0xE7E7EF
#define TABLE_BOTTOM 0xCDD3E1
#define TABLE_LINE 0xA9B1C4
#define TABLE_HEAD 0xDDE2EE

typedef enum { VIEW_TABLE, VIEW_SUBJECTS, VIEW_TOC, VIEW_TEXT } reader_view_t;
typedef struct {
  reader_view_t view;
  int row, col;
  int subject;
  int toc;
  int top;
  int line;
  int konami;
  uint64_t prev;
  np_config_t cfg;
} reader_t;

static const eadk_key_t konami[8] = {
    eadk_key_up, eadk_key_up, eadk_key_down, eadk_key_down,
    eadk_key_left, eadk_key_right, eadk_key_left, eadk_key_right,
};

static void draw_header(const char *title, const char *sub) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, DOCS_TOP, DOCS_BOTTOM);
  gfx_text(&np_font_title, 16, 25, title, 0xFFFF, 256);
  gfx_text(&np_font_small, 16, 42, sub, 0xFFFF, 190);
}

static void col_label(int col, char out[8]) {
  char tmp[8];
  int n = 0;
  unsigned v = (unsigned)col + 1;
  do {
    unsigned r = (v - 1) % 26;
    tmp[n++] = (char)('A' + r);
    v = (v - 1) / 26;
  } while (v && n < 7);
  for (int i = 0; i < n; i++) out[i] = tmp[n - 1 - i];
  out[n] = 0;
}

static void row_label(int row, char out[12]) {
  char tmp[12];
  int n = 0;
  unsigned v = (unsigned)row + 1;
  do {
    tmp[n++] = (char)('0' + (v % 10));
    v /= 10;
  } while (v && n < 11);
  for (int i = 0; i < n; i++) out[i] = tmp[n - 1 - i];
  out[n] = 0;
}

static void table_scene(void *ctx) {
  reader_t *r = ctx;
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, TABLE_TOP, TABLE_BOTTOM);
  gfx_text(&np_font_title, 14, 24, "Matrices", gfx_rgb(0x545B6E), 256);
  const int x0 = 14, y0 = 46;
  const int rh = 24, cw = 44, row_w = 28;
  const int rows = 7, cols = 6;
  int row0 = r->row - rows / 2;
  int col0 = r->col - cols / 2;
  if (row0 < 0) row0 = 0;
  if (col0 < 0) col0 = 0;

  gfx_rrect(x0 - 4, y0 - 4, row_w + cols * cw + 8, (rows + 1) * rh + 8, 6, 0xFFFF, 180);
  gfx_fill(x0, y0, row_w + cols * cw, (rows + 1) * rh, gfx_rgb(TABLE_HEAD));

  char label[12];
  for (int c = 0; c < cols; c++) {
    int gc = col0 + c;
    int x = x0 + row_w + c * cw;
    bool sel = (gc == r->col);
    gfx_fill(x, y0, cw, rh, sel ? gfx_rgb(ACCENT) : gfx_rgb(TABLE_HEAD));
    col_label(gc, label);
    gfx_text_center(&np_font_body, x + cw / 2, y0 + 16, label, sel ? 0xFFFF : gfx_rgb(0x5A6174), 256);
  }
  for (int rr = 0; rr < rows; rr++) {
    int gr = row0 + rr;
    int y = y0 + rh + rr * rh;
    bool row_sel = gr == r->row;
    gfx_fill(x0, y, row_w, rh, row_sel ? gfx_rgb(ACCENT) : gfx_rgb(TABLE_HEAD));
    row_label(gr, label);
    gfx_text_center(&np_font_body, x0 + row_w / 2, y + 16, label, row_sel ? 0xFFFF : gfx_rgb(0x5A6174), 256);
    for (int c = 0; c < cols; c++) {
      int gc = col0 + c;
      int x = x0 + row_w + c * cw;
      bool sel = gr == r->row && gc == r->col;
      gfx_fill(x, y, cw, rh, sel ? gfx_rgb(ACCENT) : 0xFFFF);
    }
  }

  for (int c = 0; c <= cols; c++) {
    int x = x0 + row_w + c * cw;
    gfx_fill(x, y0, 1, (rows + 1) * rh, gfx_rgb(TABLE_LINE));
  }
  for (int rr = 0; rr <= rows + 1; rr++) {
    int y = y0 + rr * rh;
    gfx_fill(x0, y, row_w + cols * cw, 1, gfx_rgb(TABLE_LINE));
  }
  gfx_fill(x0 + row_w, y0, 1, (rows + 1) * rh, gfx_rgb(TABLE_LINE));

  gfx_text(&np_font_small, 12, 224, "Fleches : cellule   Pi : matieres   Home : quitter", gfx_rgb(0x525A6B), 220);
}

static void subjects_scene(void *ctx) {
  reader_t *r = ctx;
  draw_header("Documents", "3 matieres");
  for (int i = 0; i < (int)np_subject_count; i++) {
    int y = 76 + i * 40;
    bool sel = i == r->subject;
    if (sel) gfx_rrect(10, y - 22, 300, 34, 8, gfx_rgb(ACCENT), 220);
    gfx_text(&np_font_body, 22, y, np_subjects[i].title, 0xFFFF, 256);
  }
  gfx_text(&np_font_small, 16, 218, "Haut/Bas : choisir   OK : sommaire   Back : tableur", 0xFFFF, 190);
}

static const np_subject_t *current_subject(reader_t *r) {
  return &np_subjects[r->subject];
}

static int toc_count(reader_t *r) {
  return (int)current_subject(r)->chapter_count + 1;
}

static int toc_offset(reader_t *r, int idx) {
  if (idx <= 0) return 0;
  return (int)current_subject(r)->chapters[idx - 1].offset;
}

static const char *toc_title(reader_t *r, int idx) {
  if (idx == 0) return "Tout le cours";
  return current_subject(r)->chapters[idx - 1].title;
}

static void toc_scene(void *ctx) {
  reader_t *r = ctx;
  const np_subject_t *s = current_subject(r);
  draw_header(s->title, "Sommaire");
  const int visible = 4;
  int count = toc_count(r);
  if (r->toc < r->top) r->top = r->toc;
  if (r->toc > r->top + visible - 1) r->top = r->toc - (visible - 1);
  if (r->top > count - visible) r->top = NP_MAX(0, count - visible);

  for (int i = 0; i < visible; i++) {
    int idx = r->top + i;
    if (idx >= count) break;
    int y = 76 + i * 38;
    bool sel = idx == r->toc;
    if (sel) gfx_rrect(10, y - 22, 300, 32, 8, gfx_rgb(ACCENT), 220);
    gfx_text(&np_font_body, 20, y, toc_title(r, idx), 0xFFFF, 256);
  }
  gfx_text(&np_font_small, 16, 218, "Haut/Bas : choisir   OK : ouvrir   Back : matieres", 0xFFFF, 190);
}

static void text_scene(void *ctx) {
  reader_t *r = ctx;
  const np_subject_t *s = current_subject(r);
  const np_doc_t *d = s->doc;
  draw_header(s->title, toc_title(r, r->toc));
  int y = 67;
  for (int i = 0; i < 7 && r->line + i < (int)d->count; i++) {
    const char *line = d->lines[r->line + i];
    if (*line) gfx_text(&np_font_small, 12, y, line, 0xFFFF, 245);
    y += 23;
  }
  gfx_text(&np_font_small, 12, 228, "Haut/Bas : defiler   Back : sommaire   Home : quitter", 0xFFFF, 180);
}

static eadk_key_t secret_key(uint8_t secret) {
  switch (secret) {
    case NP_SECRET_XNT:
      return eadk_key_xnt;
    case NP_SECRET_VAR:
      return eadk_key_var;
    case NP_SECRET_TOOLBOX:
      return eadk_key_toolbox;
    case NP_SECRET_PI:
      return eadk_key_pi;
    case NP_SECRET_SQRT:
      return eadk_key_sqrt;
    case NP_SECRET_MENU:
      return eadk_key_toolbox;
    default:
      return (eadk_key_t)-1;
  }
}

static bool open_subjects(reader_t *r, uint64_t hit) {
#define HIT(key) ((hit >> (key)) & 1ull)
  if (HIT(eadk_key_pi)) return true;
  eadk_key_t sk = secret_key(r->cfg.secret);
  if (sk != (eadk_key_t)-1 && HIT(sk)) return true;
  if (r->cfg.secret == NP_SECRET_KONAMI && hit) {
    if (hit == (1ull << konami[r->konami])) r->konami++;
    else r->konami = hit == (1ull << eadk_key_up) ? (r->konami == 2 ? 2 : 1) : 0;
    if (r->konami == 8) {
      r->konami = 0;
      return true;
    }
  } else if (hit) {
    r->konami = 0;
  }
#undef HIT
  return false;
}

int np_home(int *selected, bool returning) {
  (void)selected;
  (void)returning;
  ui_init();
  reader_t r = {VIEW_TABLE, 0, 0, 0, 0, 0, 0, 0, 0, {0}};
  np_config_load(&r.cfg);
  r.prev = eadk_keyboard_scan();
  ui_keys_t keys = {np_keys(), 0, 0};
  np_wait_release();

  for (;;) {
    if (r.view == VIEW_TABLE) ui_frame(table_scene, &r);
    else if (r.view == VIEW_SUBJECTS) ui_frame(subjects_scene, &r);
    else if (r.view == VIEW_TOC) ui_frame(toc_scene, &r);
    else ui_frame(text_scene, &r);

    uint64_t raw = eadk_keyboard_scan();
    uint64_t hit = raw & ~r.prev;
    r.prev = raw;

    uint32_t p = ui_poll(&keys);
    if (p & K_HOME) return -2;

    if (r.view == VIEW_TABLE) {
      if (p & K_UP) r.row = NP_MAX(0, r.row - 1);
      if (p & K_DOWN) r.row++;
      if (p & K_LEFT) r.col = NP_MAX(0, r.col - 1);
      if (p & K_RIGHT) r.col++;
      if (open_subjects(&r, hit)) {
        r.view = VIEW_SUBJECTS;
        r.subject = 0;
        r.toc = 0;
        r.top = 0;
        r.line = 0;
      }
    } else if (r.view == VIEW_SUBJECTS) {
      if (p & K_UP) r.subject = NP_MAX(0, r.subject - 1);
      if (p & K_DOWN) r.subject = NP_MIN((int)np_subject_count - 1, r.subject + 1);
      if (p & K_OK) {
        r.view = VIEW_TOC;
        r.toc = 0;
        r.top = 0;
      }
      if (p & K_BACK) r.view = VIEW_TABLE;
    } else if (r.view == VIEW_TOC) {
      int count = toc_count(&r);
      if (p & K_UP) r.toc = NP_MAX(0, r.toc - 1);
      if (p & K_DOWN) r.toc = NP_MIN(count - 1, r.toc + 1);
      if (p & K_OK) {
        r.line = toc_offset(&r, r.toc);
        r.view = VIEW_TEXT;
      }
      if (p & K_BACK) r.view = VIEW_SUBJECTS;
    } else {
      const np_doc_t *d = current_subject(&r)->doc;
      int max = (int)d->count - 7;
      if (max < 0) max = 0;
      if (p & K_UP) r.line = NP_MAX(0, r.line - 1);
      if (p & K_DOWN) r.line = NP_MIN(max, r.line + 1);
      if (p & K_BACK) r.view = VIEW_TOC;
    }
    np_sleep(20);
  }
}
