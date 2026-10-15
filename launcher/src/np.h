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

/* ---- storage (storage.c): Epsilon's record file system */
bool np_storage_delete(const char *name);   /* true if gone (or never there) */
uint32_t np_storage_record_size(const char *name);
/* a record's content (NULL if none), and writing one: for the games too */
const uint8_t *np_file_read(const char *name, uint32_t *len);
bool np_file_write(const char *name, const void *data, uint32_t len);
/* deletes a game's progress (np_game_t.progress); true if all of it is gone */
bool np_reset_game(int game);

/* NumPlay's own settings, in the file numplay.set */
enum { NP_SECRET_XNT, NP_SECRET_VAR, NP_SECRET_TOOLBOX, NP_SECRET_PI, NP_SECRET_SQRT, NP_SECRET_MENU, NP_SECRET_KONAMI, NP_SECRET_COUNT };
typedef struct {
  bool disguise;  /* start as Matrices, a calculator app; NumPlay opens through the secret */
  uint8_t secret; /* NP_SECRET_* */
  bool hint;      /* Matrices names the secret in light gray, bottom left */
} np_config_t;
void np_config_load(np_config_t *c);
bool np_config_save(const np_config_t *c);
bool np_bg_load(int *mode);   /* the home screen's live background chosen last; false when none was saved */
bool np_bg_save(int mode);
/* A copy of every save in numplay_saves.py, a Python script: the only kind of
   file the NumWorks installer keeps when NumPlay is updated. */
void np_progress_backup(void);
void np_progress_restore(void); /* only when none of the saves are left */
/* Matrices, the disguise: true when the secret was used (open NumPlay), false to quit */
int np_sheet(void);
int np_home(int *selected, bool returning);


#endif
