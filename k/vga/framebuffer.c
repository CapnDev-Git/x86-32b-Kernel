#include "framebuffer.h"

#include <k/graphics_colors.h>
#include <k/kstd.h>
#include <k/types.h>
#include <stddef.h>
#include <string.h>

#include "libvga.h"
#include "vga.h"

#define LINES 25
#define COLUMNS 80

int setvideo(int mode) {
  if (mode == VIDEO_GRAPHIC)
    libvga_switch_mode13h();
  else if (mode == VIDEO_TEXT)
    libvga_switch_mode3h();
  else
    return -1;
  return 0;
}

void swap_frontbuffer(const void *buffer) {
  char *frame = libvga_get_framebuffer();
  memcpy(frame, buffer, GRAPHIC_WIDTH * GRAPHIC_HEIGHT);
}

void write_fb(char *fb, const char *buf, size_t *line, u8 color) {
  size_t len = strlen(buf);

  // Go through each character in the buffer and write it to the framebuffer at
  // the current line and column position with provided color
  for (size_t i = 0; i < len; i++) {
    fb[(i + (*line) * COLUMNS) * 2] = buf[i];    // Character
    fb[(i + (*line) * COLUMNS) * 2 + 1] = color; // Default color
  }

  // Move to the next line for the next write to the framebuffer
  (*line)++;
}

void clear_fb(char *fb) {
  // Go through each character and set it to a space with the default color
  for (size_t i = 0; i < COLUMNS * LINES; i++) {
    fb[i * 2] = ' ';      // Space character
    fb[i * 2 + 1] = 0x07; // Default color
  }
}

void init_fb(char *fb, size_t *line) {
  // Flush the framebuffer
  clear_fb(fb);

  // Write the initialization message
  write_fb(fb, "Framebuffer initialized", line, FRAMEBUFFER_YELLOW);
}
