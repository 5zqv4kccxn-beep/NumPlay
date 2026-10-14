/* NumPlay home: a clean spreadsheet-like dashboard and embedded course reader. */
#include "ui.h"
#include "reader_docs.h"
#include "sys.h"
#include <string.h>

#define NAVY 0x101923
#define BLUE 0x24364A
#define TEAL 0x41C7A1
#define GOLD 0xF4C95D

typedef enum { HOME_SHEET, SUBJECTS, CHAPTERS, READING } screen_t;
typedef struct { screen_t screen; int subject; int chapter; int line; } reader_t;
static const char *subjects[] = {"Chimie", "Maths", "Physique"};
static const int subject_docs[][2] = {{0,1},{2,-1},{2,-1}};
static const int subject_counts[] = {2,1,1};

static void header(const char *title) {
  gfx_vgrad(0,0,SCREEN_W,SCREEN_H,BLUE,NAVY);
  gfx_text(&np_font_title,16,25,"NumPlay",0xFFFF,256);
  gfx_text(&np_font_small,16,43,title,0xFFFF,210);
}
static void sheet_scene(void *ctx) {
  (void)ctx; header("Tableur d'etude");
  gfx_rrect(12,58,296,137,8,0xFFFF,245);
  int x0=12,y0=58,cw=42,rh=22;
  for(int c=0;c<7;c++) { char t[2]={(char)('A'+c),0}; gfx_text_center(&np_font_small,x0+c*cw+cw/2,y0+15,t,BLUE,220); }
  for(int r=0;r<6;r++) { char t[2]={(char)('1'+r),0}; gfx_text_center(&np_font_small,x0+12,y0+rh*(r+1)+15,t,BLUE,180); }
  for(int c=0;c<7;c++) for(int r=0;r<6;r++) gfx_rrect(x0+cw*(c+1),y0+rh*(r+1),cw,rh,0,0xD8E7EA,150);
  gfx_rrect(55,82,40,22,2,TEAL,180); gfx_text(&np_font_small,62,98,"1",0xFFFF,256);
  gfx_text(&np_font_small,18,218,"Pi : matieres   OK : ouvrir   Home : quitter",0xFFFF,210);
}
static void subjects_scene(void *ctx) {
  reader_t *r=ctx; header("Matieres");
  for(int i=0;i<3;i++) { int y=78+i*40; bool sel=i==r->subject; if(sel) gfx_rrect(20,y-20,280,30,7,gfx_rgb(TEAL),230); gfx_text(&np_font_body,38,y,subjects[i],0xFFFF,256); }
  gfx_text(&np_font_small,18,218,"Haut/Bas : choisir   OK : sommaire",0xFFFF,210);
}
static void chapters_scene(void *ctx) {
  reader_t *r=ctx; header(subjects[r->subject]);
  int n=subject_counts[r->subject];
  for(int i=0;i<n;i++) { int y=78+i*40; bool sel=i==r->chapter; if(sel) gfx_rrect(20,y-20,280,30,7,gfx_rgb(GOLD),230); gfx_text(&np_font_body,34,y,np_docs[subject_docs[r->subject][i]].title,0xFFFF,250); }
  gfx_text(&np_font_small,18,205,"OK : chapitre   Droite : tout le cours",0xFFFF,200);
  gfx_text(&np_font_small,18,221,"Gauche : matieres",0xFFFF,180);
}
static void reading_scene(void *ctx) {
  reader_t *r=ctx; int d=subject_docs[r->subject][r->chapter]; header(np_docs[d].title);
  int max=(int)np_docs[d].count-7; if(max<0) max=0; int y=67;
  for(int i=0;i<7 && r->line+i<(int)np_docs[d].count;i++) { const char *s=np_docs[d].lines[r->line+i]; if(*s) gfx_text(&np_font_small,12,y,s,0xFFFF,245); y+=23; }
  gfx_text(&np_font_small,12,228,"Haut/Bas : defiler   Gauche : retour",0xFFFF,190);
  (void)max;
}
int np_home(int *selected, bool returning) {
  (void)selected; (void)returning; ui_init(); reader_t r={HOME_SHEET,0,0,0};
  ui_keys_t keys={np_keys(),0,0}; np_wait_release();
  for(;;) {
    ui_frame(r.screen==HOME_SHEET?sheet_scene:r.screen==SUBJECTS?subjects_scene:r.screen==CHAPTERS?chapters_scene:reading_scene,&r);
    uint32_t p=ui_poll(&keys); if(p&K_HOME) return -2;
    if(r.screen==HOME_SHEET) { if(p&K_PI) r.screen=SUBJECTS; else if(p&K_OK) r.screen=SUBJECTS; }
    else if(r.screen==SUBJECTS) { if(p&K_UP) r.subject=NP_MAX(0,r.subject-1); if(p&K_DOWN) r.subject=NP_MIN(2,r.subject+1); if(p&K_OK) {r.chapter=0;r.screen=CHAPTERS;} if(p&K_BACK) r.screen=HOME_SHEET; }
    else if(r.screen==CHAPTERS) { int n=subject_counts[r.subject]; if(p&K_UP) r.chapter=NP_MAX(0,r.chapter-1); if(p&K_DOWN) r.chapter=NP_MIN(n-1,r.chapter+1); if(p&(K_OK|K_RIGHT)) {r.line=0;r.screen=READING;} if(p&K_LEFT||p&K_BACK) r.screen=SUBJECTS; }
    else { int d=subject_docs[r.subject][r.chapter], max=NP_MAX(0,(int)np_docs[d].count-7); if(p&K_UP) r.line=NP_MAX(0,r.line-1); if(p&K_DOWN) r.line=NP_MIN(max,r.line+1); if(p&K_LEFT||p&K_BACK) r.screen=CHAPTERS; }
    np_sleep(20);
  }
}
