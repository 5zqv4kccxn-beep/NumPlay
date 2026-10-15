/* NumPlay launcher: shared definitions. */
#ifndef NP_H
#define NP_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint16_t color_t;
#define RGB(r, g, b) ((color_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))
#define HEX(v) RGB(((v) >> 16) & 0xFF, ((v) >> 8) & 0xFF, (v) & 0xFF)
#define SCREEN_W 320
#define SCREEN_H 240
#define NP_MIN(a, b) ((a) < (b) ? (a) : (b))
#define NP_MAX(a, b) ((a) > (b) ? (a) : (b))
#define NP_CLAMP(v, lo, hi) ((v) < (lo) ? (lo) : (v) > (hi) ? (hi) : (v))

/* Keys, independent of the calculator's numbering. The number pad doubles
 * as arrows (8 4 6 2) and 5 as OK. */
enum {
  K_LEFT = 1, K_RIGHT = 2, K_UP = 4, K_DOWN = 8, K_OK = 16, K_BACK = 32, K_HOME = 64, K_BACKSPACE = 128, K_PI = 256, K_DIGIT0 = 512, K_DIGIT1 = 1024, K_DIGIT2 = 2048, K_DIGIT3 = 4096, K_DIGIT4 = 8192, K_DIGIT5 = 16384, K_DIGIT6 = 32768, K_DIGIT7 = 65536, K_DIGIT8 = 131072, K_DIGIT9 = 262144
};

/* ---- platform (platform.c) */
uint32_t np_keys(void);           /* keys held right now */
uint32_t np_millis(void);
void np_sleep(unsigned ms);
void np_vblank(void);
void np_push(int x, int y, int w, int h, const color_t *px);
void np_fill_screen(int x, int y, int w, int h, color_t c);
/* Waits until every key is released (after a game, or before quitting). */
void np_wait_release(void);

/* ---- one arena for everything big: the running game's RAM, or the
 * launcher's buffers while no game runs */
extern uint8_t np_arena[];
extern const uint32_t np_arena_size;
uint32_t np_arena_room(void);     /* the arena and the RAM after it, up to the end of the app's */
void *np_alloc(uint32_t size);    /* bump allocator in the arena */
void np_alloc_reset(void);

/* The Tableur application has no game catalogue or game storage API. */

int np_sheet(void);
int np_home(int *selected, bool returning);


#endif
