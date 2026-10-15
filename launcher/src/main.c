/* NumPlay: embedded course reader. */
#include <eadk.h>
#include "np.h"
#include "sys.h"

#if PLATFORM_DEVICE && !NP_SIMULATOR
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;
/* Tells the games (games/common/epsilon_app.h) that the launcher takes care
 * of Home and of the RAM. */
const uint8_t numplay_launcher[1] = {1};

/* The games read the keyboard through these (the build renames their calls),
 * so NumPlay knows when a game ended because of Home or On/Off. */
static bool home_pressed;
eadk_keyboard_state_t np_keyboard_scan(void) {
  eadk_keyboard_state_t s = eadk_keyboard_scan();
  if (s & ((1ull << eadk_key_home) | (1ull << eadk_key_on_off))) home_pressed = true;
  return s;
}
eadk_event_t np_event_get(int32_t *timeout) {
  eadk_event_t e = eadk_event_get(timeout);
  if (e == 6 || e == 8) home_pressed = true; /* Home, On/Off */
  return e;
}
#else
static bool home_pressed;
#endif

int main(void) {
  np_session_begin();
  np_wait_release();
  np_config_t cfg; np_config_load(&cfg);
  (void)cfg;
  np_sheet();
  return np_session_end();
}
