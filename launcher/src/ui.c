#include "ui.h"

void ui_init(void) { np_alloc_reset(); gfx_init(); }

/* ---- input */
uint32_t ui_poll(ui_keys_t *k) {
  uint32_t now = np_millis(), keys = np_keys();
  uint32_t arrows = K_LEFT | K_RIGHT | K_UP | K_DOWN;
  uint32_t pressed = keys & ~k->held;
  if (pressed) {
    k->since = now;
    k->next = now + 380;
  } else if ((keys & arrows) && (int32_t)(now - k->next) >= 0) {
    pressed = keys & arrows;
    k->next = now + 120;
  }
  k->held = keys;
  return pressed;
}

/* ---- motion */
float ui_ease_out(float t) {
  t = NP_CLAMP(t, 0.f, 1.f);
  float u = 1 - t;
  return 1 - u * u * u;
}

float ui_ease_in_out(float t) {
  t = NP_CLAMP(t, 0.f, 1.f);
  return t < 0.5f ? 4 * t * t * t : 1 - (-2 * t + 2) * (-2 * t + 2) * (-2 * t + 2) / 2;
}

/* Exponential approach, independent of the frame rate. */
float ui_approach(float v, float target, float dt, float tau) {
  float k = dt / (tau + dt);
  float out = v + (target - v) * k;
  float d = out - target;
  return (d < 0.001f && d > -0.001f) ? target : out;
}

void ui_frame(gfx_scene_t scene, void *ctx) {
  np_vblank();
  gfx_render(scene, ctx, 0, SCREEN_H);
}

/* ---- pieces */
void ui_button(int x, int y, int w, int h, const char *label, bool selected, uint32_t accent, int a) {
  if (selected) {
    gfx_rrect(x, y + 2, w, h, h / 2, 0, a / 4);
    gfx_rrect(x, y, w, h, h / 2, gfx_rgb(accent), a);
    gfx_text_center(&np_font_body, x + w / 2, y + h / 2 + 4, label, 0xFFFF, a);
  } else {
    gfx_rrect(x, y, w, h, h / 2, 0xFFFF, a * 40 / 256);
    gfx_text_center(&np_font_body, x + w / 2, y + h / 2 + 4, label, 0xFFFF, a * 200 / 256);
  }
}

void ui_key_hint(int x, int baseline, const char *key, const char *label, color_t c) {
  int kw = gfx_text_width(&np_font_small, key) + 8;
  gfx_rrect(x, baseline - 10, kw, 13, 4, c, 70);
  gfx_text(&np_font_small, x + 4, baseline, key, c, 256);
  gfx_text(&np_font_small, x + kw + 4, baseline, label, c, 200);
}
