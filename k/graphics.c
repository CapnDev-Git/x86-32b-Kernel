#include "graphics.h"

#include <stddef.h>
#include <string.h>

#define LINES 25
#define COLUMNS 80

void write_framebuffer(char *fb, const char *buf, size_t *line) {
  size_t len = strlen(buf);

  for (size_t i = 0; i < len; i++) {
    fb[(i + (*line) * COLUMNS) * 2] = buf[i];
    fb[(i + (*line) * COLUMNS) * 2 + 1] = 0x07;
  }
  (*line)++;
}

void init_framebuffer(char *fb, size_t *line) {
  // Clear the framebuffer
  for (size_t i = 0; i < COLUMNS * LINES; i++) {
    fb[i * 2] = ' ';
    fb[i * 2 + 1] = 0x07;
  }

  // Write the initialization message
  write_framebuffer(fb, "Kernel Initialized!", line);
}
