/* NumPlay spreadsheet: 10 columns x 30 rows, deliberately self-contained. */
#include "ui.h"
#include "reader_docs.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>

#define COLS 10
#define ROWS 30
#define CELL_MAX 64
#define NAVY 0x101923
#define BLUE 0x24364A
#define TEAL 0x41C7A1
#define GOLD 0xF4C95D
static char cells[ROWS][COLS][CELL_MAX];
static float values[ROWS][COLS];
static unsigned char state[ROWS][COLS];
static int cur_r,cur_c,row0,col0,editing,menu;
static char edit[CELL_MAX];
static ui_keys_t keys;

static int colnum(const char **p) { int c=0; while(isalpha((unsigned char)**p)){ c=c*26+(toupper((unsigned char)**p)-'A'+1); (*p)++; } return c-1; }
static void skip(const char **p){ while(**p==' '||**p=='\t')(*p)++; }
static float expr(const char **p,int *err);
static float cellval(int r,int c,int *err) {
  if(r<0||r>=ROWS||c<0||c>=COLS){*err=1;return 0;}
  if(state[r][c]==1){*err=1;return 0;} if(state[r][c]==2)return values[r][c];
  state[r][c]=1; const char *s=cells[r][c]; char *e;
  if(!*s) values[r][c]=0;
  else if(*s!='='){ values[r][c]=strtof(s,&e); if(e==s)*err=1; }
  else { const char *p=s+1; values[r][c]=expr(&p,err); skip(&p); if(*p)*err=1; }
  state[r][c]=2; return values[r][c];
}
static float range(const char **p,int *err,int fn) {
  skip(p); int c1=colnum(p), r1=0; while(isdigit((unsigned char)**p)){r1=r1*10+*(*p)++-'0';} r1--;
  skip(p); int c2=c1,r2=r1; if(**p==':'){(*p)++; skip(p); c2=colnum(p); r2=0; while(isdigit((unsigned char)**p)){r2=r2*10+*(*p)++-'0';} r2--;}
  if(c1<0||c2<0||r1<0||r2<0){*err=1;return 0;} float out=fn==1?0:fn==2?999999.f:-999999.f; int n=0;
  for(int r=NP_MIN(r1,r2);r<=NP_MAX(r1,r2);r++)for(int c=NP_MIN(c1,c2);c<=NP_MAX(c1,c2);c++){float v=cellval(r,c,err); if(fn==1)out+=v; else if(fn==2&&v<out)out=v; else if(fn==3&&v>out)out=v; n++;}
  return fn==4?(n?out/n:0):out;
}
static float factor(const char **p,int *err){ skip(p); if(**p=='('){(*p)++;float v=expr(p,err);skip(p);if(**p==')')(*p)++;else *err=1;return v;} if(**p=='-'){(*p)++;return -factor(p,err);} if(isalpha((unsigned char)**p)){const char *q=*p; int c=colnum(&q); int r=0; while(isdigit((unsigned char)*q)){r=r*10+*q++-'0';} if(c>=0&&r){*p=q;return cellval(r-1,c,err);} *err=1;return 0;} char *e;float v=strtof(*p,&e);if(e==*p)*err=1;*p=e;return v; }
static float term(const char **p,int *err){float v=factor(p,err);for(;;){skip(p);char op=**p;if(op!='*'&&op!='/')return v;(*p)++;float b=factor(p,err);if(op=='*')v*=b;else if(b==0)*err=1;else v/=b;}}
static float expr(const char **p,int *err){skip(p); if(!strncasecmp(*p,"SOMME(",6)){*p+=6;float v=range(p,err,1);if(**p==')')(*p)++;else *err=1;return v;} if(!strncasecmp(*p,"MOYENNE(",8)){*p+=8;float v=range(p,err,4);if(**p==')')(*p)++;else *err=1;return v;} if(!strncasecmp(*p,"MIN(",4)){*p+=4;float v=range(p,err,2);if(**p==')')(*p)++;else *err=1;return v;} if(!strncasecmp(*p,"MAX(",4)){*p+=4;float v=range(p,err,3);if(**p==')')(*p)++;else *err=1;return v;} float v=term(p,err);for(;;){skip(p);char op=**p;if(op!='+'&&op!='-')return v;(*p)++;float b=term(p,err);v=op=='+'?v+b:v-b;}}
static void recalc(void){memset(state,0,sizeof state);for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++){int e=0;cellval(r,c,&e);if(e)values[r][c]=NAN;}}
static void putkey(char ch){size_t n=strlen(edit);if(n+1<CELL_MAX){edit[n]=ch;edit[n+1]=0;}}
static void header(void){gfx_vgrad(0,0,SCREEN_W,SCREEN_H,BLUE,NAVY);gfx_text(&np_font_title,12,25,"NumPlay",0xFFFF,256);gfx_text(&np_font_small,112,24,"TABLEUR  A-J / 1-30",0xFFFF,220);gfx_rrect(10,31,300,24,4,0xFFFFFF,245);gfx_text(&np_font_small,16,48,edit,BLUE,256);}
static void scene(void *x){(void)x;header();int x0=10,y0=61,cw=34,rh=17;gfx_rrect(x0,y0,300,150,2,0xFFFFFF,245);for(int c=0;c<8&&col0+c<COLS;c++){char s[2]={(char)('A'+col0+c),0};gfx_text_center(&np_font_small,x0+22+c*cw,y0+13,s,BLUE,256);}for(int r=0;r<9&&row0+r<ROWS;r++){char s[4];snprintf(s,sizeof s,"%d",row0+r+1);gfx_text_right(&np_font_small,x0+18,y0+29+r*rh,s,BLUE,220);for(int c=0;c<8&&col0+c<COLS;c++){int cc=col0+c,rr=row0+r;int xx=x0+22+c*cw; if(rr==cur_r&&cc==cur_c)gfx_fill(xx,y0+17+r*rh,cw,rh,TEAL);char out[16];if(cells[rr][cc][0]=='='){if(isnan(values[rr][cc]))strcpy(out,"#ERR");else snprintf(out,sizeof out,"%.5g",values[rr][cc]);}else snprintf(out,sizeof out,"%.12s",cells[rr][cc]);gfx_text_center(&np_font_small,xx+cw/2,y0+29+r*rh,out,rr==cur_r&&cc==cur_c?0xFFFF:BLUE,256);}}gfx_text(&np_font_small,10,225,menu?"Fonctions: =  SOMME(  MOYENNE(  MIN(  MAX(  A1:B3":"Fleches: naviguer  OK/EXE: editer/valider  DEL: effacer  PI: fonctions",0xFFFF,210);}
static void menu_key(void){
  static const char *const token[] = {"+","-","*","/","(",")",":","A","B","C","D","E","F","G","H","I","J"};
  menu=(menu+1)%18; edit[0]=0; strcpy(edit,token[menu-1]);
}
int np_sheet(void){memset(cells,0,sizeof cells);cur_r=cur_c=row0=col0=editing=menu=0;edit[0]=0;np_wait_release();keys=(ui_keys_t){0,0,0};for(;;){recalc();ui_frame(scene,0);uint32_t p=ui_poll(&keys);if(p&K_HOME)return -2;if(!editing){if(p&K_LEFT)cur_c=NP_MAX(0,cur_c-1);if(p&K_RIGHT)cur_c=NP_MIN(COLS-1,cur_c+1);if(p&K_UP)cur_r=NP_MAX(0,cur_r-1);if(p&K_DOWN)cur_r=NP_MIN(ROWS-1,cur_r+1);if(cur_r<row0)row0=cur_r;if(cur_r>=row0+9)row0=cur_r-8;if(cur_c<col0)col0=cur_c;if(cur_c>=col0+8)col0=cur_c-7;if(p&K_BACKSPACE){cells[cur_r][cur_c][0]=0;}if(p&K_PI)menu_key();if(p&K_OK){editing=1;strcpy(edit,cells[cur_r][cur_c]);}}else{if(p&K_BACK){editing=0;edit[0]=0;}if(p&K_BACKSPACE){size_t n=strlen(edit);if(n)edit[n-1]=0;}if(p&K_OK){strncpy(cells[cur_r][cur_c],edit,CELL_MAX-1);cells[cur_r][cur_c][CELL_MAX-1]=0;editing=0;edit[0]=0;}if(p&K_PI)menu_key();for(int d=0;d<10;d++) if(p&(K_DIGIT0<<d)) putkey((char)('0'+d));} if(p&(K_DIGIT0|K_DIGIT1|K_DIGIT2)){static char sec[4];static int sl;char d=p&K_DIGIT2?'2':p&K_DIGIT1?'1':'0';if(sl<4)sec[sl++]=d;else{memmove(sec,sec+1,3);sec[3]=d;}if(sl==4&&!memcmp(sec,"2011",4)){sl=0;edit[0]=0;editing=0;np_home(0,true);np_wait_release();}}np_sleep(20);}}
