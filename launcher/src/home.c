/* NumPlay home and navigable course reader. */
#include "ui.h"
#include "reader_docs.h"
#include "sys.h"
#include <string.h>

typedef enum { HOME_SHEET, SUBJECTS, TOC, READING } screen_t;
typedef struct { screen_t screen; int subject; int toc; int line; } reader_t;
static const char *subjects[] = {"Corps purs et melanges", "Solutions / dissolution", "Maths-Physique seconde"};
static const int subject_docs[] = {0, 1, 2};
static void header(const char *title) {
  gfx_vgrad(0,0,SCREEN_W,SCREEN_H,0x24364A,0x101923);
  gfx_text(&np_font_title,16,25,"NumPlay",0xFFFF,256);
  gfx_text(&np_font_small,16,43,title,0xFFFF,210);
}
static void sheet_scene(void *ctx) { (void)ctx; header("Tableur d'etude"); gfx_rrect(12,58,296,137,8,0xFFFF,245); gfx_text(&np_font_small,18,218,"OK : ouvrir   2011 : cours   Home : quitter",0xFFFF,210); }
static void subjects_scene(void *ctx) { reader_t *r=ctx; header("Cours"); for(int i=0;i<3;i++){int y=78+i*40; if(i==r->subject) gfx_rrect(12,y-20,296,30,7,gfx_rgb(0x41C7A1),230); gfx_text(&np_font_small,22,y,subjects[i],0xFFFF,245);} gfx_text(&np_font_small,18,218,"Haut/Bas : choisir   OK : sommaire",0xFFFF,210); }
static void toc_scene(void *ctx) { reader_t *r=ctx; const np_toc_t *t=&np_tocs[subject_docs[r->subject]]; header("Sommaire"); int first=0; if(r->toc>7) first=r->toc-7; for(int i=first;i<(int)t->count && i<first+8;i++){int y=68+(i-first)*19; if(i==r->toc) gfx_rrect(8,y-14,304,18,4,gfx_rgb(0xF4C95D),230); gfx_text(&np_font_small,14,y,t->entries[i].label,0xFFFF,245);} gfx_text(&np_font_small,12,226,"Haut/Bas : choisir   OK : aller au titre",0xFFFF,190); }
static void reading_scene(void *ctx) { reader_t *r=ctx; int d=subject_docs[r->subject]; header(np_docs[d].title); int max=(int)np_docs[d].count-9; if(max<0)max=0; int y=64; for(int i=0;i<9 && r->line+i<(int)np_docs[d].count;i++){const char *s=np_docs[d].lines[r->line+i]; if(*s)gfx_text(&np_font_small,10,y,s,0xFFFF,245); y+=17;} gfx_text(&np_font_small,10,228,"Haut/Bas : defiler   Gauche : sommaire",0xFFFF,185); (void)max; }
int np_home(int *selected, bool returning) { (void)selected; (void)returning; ui_init(); reader_t r={HOME_SHEET,0,0,0}; ui_keys_t keys={np_keys(),0,0}; np_wait_release(); char seq[5]={0}; int sl=0; for(;;){ui_frame(r.screen==HOME_SHEET?sheet_scene:r.screen==SUBJECTS?subjects_scene:r.screen==TOC?toc_scene:reading_scene,&r); uint32_t p=ui_poll(&keys); if(p&K_HOME)return -2; uint32_t digits=p&(K_DIGIT0|K_DIGIT1|K_DIGIT2); if(digits){char d=(digits&K_DIGIT1)?'1':((digits&K_DIGIT2)?'2':'0'); if(sl<4)seq[sl++]=d; seq[sl]=0; if(sl==4){if(!strcmp(seq,"2011")){r.screen=SUBJECTS;r.subject=0;r.toc=0;r.line=0;}sl=0;seq[0]=0;}} if(r.screen==HOME_SHEET){if(p&K_OK)r.screen=SUBJECTS;} else if(r.screen==SUBJECTS){if(p&K_UP)r.subject=NP_MAX(0,r.subject-1);if(p&K_DOWN)r.subject=NP_MIN(2,r.subject+1);if(p&K_OK){r.toc=0;r.screen=TOC;}if(p&K_BACK)r.screen=HOME_SHEET;} else if(r.screen==TOC){const np_toc_t *t=&np_tocs[subject_docs[r.subject]];if(p&K_UP)r.toc=NP_MAX(0,r.toc-1);if(p&K_DOWN)r.toc=NP_MIN((int)t->count-1,r.toc+1);if(p&K_OK){r.line=t->entries[r.toc].line;r.screen=READING;}if(p&K_LEFT||p&K_BACK)r.screen=SUBJECTS;} else {int d=subject_docs[r.subject],max=NP_MAX(0,(int)np_docs[d].count-9);if(p&K_UP)r.line=NP_MAX(0,r.line-1);if(p&K_DOWN)r.line=NP_MIN(max,r.line+1);if(p&K_LEFT||p&K_BACK)r.screen=TOC;} np_sleep(20);}}
