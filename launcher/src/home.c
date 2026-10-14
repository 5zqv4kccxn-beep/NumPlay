/* NumPlay course reader: subjects -> generated chapter index -> text. */
#include "ui.h"
#include "reader_docs.h"
#include "sys.h"
#include <string.h>

#define DOCS_TOP 0x24364A
#define DOCS_BOTTOM 0x101923
#define ACCENT 0x41C7A1
#define MAX_CHAPTERS 64

typedef struct { int doc; int line; bool reading; bool chapters; int item; int chapter_count; int offsets[MAX_CHAPTERS]; const char *titles[MAX_CHAPTERS]; } reader_t;

static bool starts(const char *s, const char *prefix) { return strncmp(s, prefix, strlen(prefix)) == 0; }
static void build_index(reader_t *r) {
  r->chapter_count = 0;
  for (unsigned i = 0; i < np_docs[r->doc].count && r->chapter_count < MAX_CHAPTERS; i++) {
    const char *s = np_docs[r->doc].lines[i];
    /* Course headings are deliberately detected from the embedded text, so
       adding a chapter to reader_docs.h does not require another table. */
    if (starts(s, "Chapitre ") || (s[0] >= '1' && s[0] <= '9' && s[1] == '.' && s[2] == ' ')) {
      r->offsets[r->chapter_count] = (int)i;
      r->titles[r->chapter_count++] = s;
    }
  }
}
static void header(const char *title) {
  gfx_vgrad(0,0,SCREEN_W,SCREEN_H,DOCS_TOP,DOCS_BOTTOM);
  gfx_text(&np_font_title,16,25,"Cours",0xFFFF,256);
  gfx_text(&np_font_small,16,42,title,0xFFFF,190);
}
static void subjects_scene(void *ctx) {
  reader_t *r=ctx; header("Matieres");
  for (int i=0; i<(int)np_doc_count; i++) {
    int y=78+i*40; if (i==r->doc) gfx_rrect(10,y-22,300,32,8,gfx_rgb(ACCENT),220);
    gfx_text(&np_font_body,22,y,np_docs[i].title,0xFFFF,256);
  }
  gfx_text(&np_font_small,16,222,"Haut/Bas : choisir   OK : ouvrir   Back : tableur",0xFFFF,190);
}
static void chapters_scene(void *ctx) {
  reader_t *r=ctx; header(np_docs[r->doc].title);
  const int total = r->chapter_count + 1;
  for (int i=0; i<total; i++) {
    int y=70+i*28; bool sel=i==r->item;
    if (sel) gfx_rrect(10,y-18,300,24,6,gfx_rgb(ACCENT),220);
    gfx_text(&np_font_small,20,y,i==0 ? "Tout le cours" : r->titles[i-1],0xFFFF,245);
  }
  gfx_text(&np_font_small,16,228,"OK : lire   Back : matieres",0xFFFF,190);
}
static void read_scene(void *ctx) {
  reader_t *r=ctx; header(np_docs[r->doc].title);
  int first=r->line, y=67;
  for (int i=0; i<7 && first+i<(int)np_docs[r->doc].count; i++) {
    const char *s=np_docs[r->doc].lines[first+i];
    if (*s) gfx_text(&np_font_small,12,y,s,0xFFFF,245);
    y+=23;
  }
  gfx_text(&np_font_small,12,228,"Haut/Bas : defiler   Back : sommaire",0xFFFF,180);
}

int np_home(int *selected, bool returning) {
  (void)selected; (void)returning; ui_init();
  reader_t r={0,0,false,false,0,0,{0},{0}};
  build_index(&r);
  ui_keys_t keys={np_keys(),0,0}; np_wait_release();
  for (;;) {
    ui_frame(r.reading?read_scene:(r.chapters?chapters_scene:subjects_scene),&r);
    uint32_t p=ui_poll(&keys);
    if (p&K_HOME) return -2;
    if (r.reading) {
      int max=(int)np_docs[r.doc].count-7; if(max<0) max=0;
      if(p&K_UP) r.line=NP_MAX(0,r.line-1);
      if(p&K_DOWN) r.line=NP_MIN(max,r.line+1);
      if(p&K_BACK) r.reading=false;
    } else if (r.chapters) {
      int max=r.chapter_count;
      if(p&K_UP) r.item=NP_MAX(0,r.item-1);
      if(p&K_DOWN) r.item=NP_MIN(max,r.item+1);
      if(p&K_BACK) r.chapters=false;
      if(p&K_OK) { r.line = r.item==0 ? 0 : r.offsets[r.item-1]; r.reading=true; }
    } else {
      if(p&K_UP) r.doc=NP_MAX(0,r.doc-1);
      if(p&K_DOWN) r.doc=NP_MIN((int)np_doc_count-1,r.doc+1);
      if(p&K_BACK) return -3;
      if(p&K_OK) { r.chapters=true; r.item=0; build_index(&r); }
    }
    np_sleep(20);
  }
}
