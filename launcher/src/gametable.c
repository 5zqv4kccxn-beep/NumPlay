/* Empty game registry: all game modules were intentionally removed. */
#include "np.h"
const np_game_t np_games[1] = {{0}};
const int np_game_count = 0;
const uint32_t np_arena_size = 96 * 1024;
uint8_t np_arena[96 * 1024] __attribute__((aligned(8)));
