#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stddef.h>

void write_framebuffer(char *fb, const char *buf, size_t *line);
void init_framebuffer(char *fb, size_t *line);

#endif /* ! GRAPHICS_H */
