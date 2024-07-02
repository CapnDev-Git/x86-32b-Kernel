#include <graphic.h>

static void splash_screen(void) {
  struct image *img = load_image("/usr/shark/res/splash_shark.bmp");
  if (!img)
    blue_screen("Unable to load shark.bmp");

  draw_begin();
  draw_image(img, 5, 10);
  draw_text(" SharkOS   ", 160, 50, BLUE, 0);
  draw_text("powered by Capn", 160, 60, RED, 0);
  draw_text("Electif Kernel - 2023-2024", 5, 190, ORANGE, 0);
  draw_end();
}

void entry(void) {
  switch_graphic();

  while (1) {
    splash_screen();
  }

  // shouldn't be reached
  switch_text();
}
