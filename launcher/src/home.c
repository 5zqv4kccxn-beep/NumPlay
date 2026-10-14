/* NumPlay reader: the secret entry point opens three embedded study documents. */
#include "ui.h"
#include "reader_docs.h"
#include "sys.h"
#include <string.h>

#define DOCS_TOP 0x24364A
#define DOCS_BOTTOM 0x101923
#define ACCENT 0x41C7A1

typedef struct { int doc; int line; bool reading; } reader_t;
static void draw_header(const char *title) {
  gfx_vgrad(0,0,SCREEN_W,SCREEN_H,DOCS_TOP,DOCS_BOTTOM);
  gfx_text(&np_font_title,16,25,"Documents",0xFFFF,256);
  gfx_text(&np_font_small,16,42,title,0xFFFF,190);
}
static void list_scene(void *ctx) {
  reader_t *r=ctx; draw_header("Cours embarques");
  for (int i=0;i<(int)np_doc_count;i++) {
    int y=70+i*42; bool sel=i==r->doc;
    if(sel) gfx_rrect(10,y-22,300,34,8,gfx_rgb(ACCENT),220);
    gfx_text(&np_font_body,22,y,"",0xFFFF,256);
    gfx_text(&np_font_body,22,y,np_docs[i].title,0xFFFF,256);
  }
  gfx_text(&np_font_small,16,218,"Haut/Bas : choisir   OK : lire   Home : quitter",0xFFFF,190);
}
static void read_scene(void *ctx) {
  reader_t *r=ctx; draw_header(np_docs[r->doc].title);
  int first=r->line, y=67;
  for(int i=0;i<7 && first+i<(int)np_docs[r->doc].count;i++) {
    const char *s=np_docs[r->doc].lines[first+i];
    if(*s) gfx_text(&np_font_small,12,y,s,0xFFFF,245);
    y+=23;
  }
  gfx_text(&np_font_small,12,228,"Haut/Bas : defiler   Back : liste   Home : quitter",0xFFFF,180);
}
int np_home(int *selected, bool returning) {
  (void)selected; (void)returning; ui_init(); reader_t r={0,0,false};
  ui_keys_t keys={np_keys(),0,0}; np_wait_release();
  for (;;) {
    ui_frame(r.reading?read_scene:list_scene,&r);
    uint32_t p=ui_poll(&keys);
    if(p&K_HOME) return -2;
    if(!r.reading) {
      if(p&K_UP) r.doc=NP_MAX(0,r.doc-1);
      if(p&K_DOWN) r.doc=NP_MIN((int)np_doc_count-1,r.doc+1);
      if(p&K_OK) { r.reading=true; r.line=0; }
    } else {
      int max=(int)np_docs[r.doc].count-7; if(max<0) max=0;
      if(p&K_UP) r.line=NP_MAX(0,r.line-1);
      if(p&K_DOWN) r.line=NP_MIN(max,r.line+1);
      if(p&K_BACK) r.reading=false;
    }
    np_sleep(20);
  }
}
