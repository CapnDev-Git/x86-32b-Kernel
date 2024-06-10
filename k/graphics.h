#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <k/types.h>
#include <stddef.h>

/**
 * \brief Write a buffer to the framebuffer
 * \param fb   Framebuffer to write to
 * \param buf  Buffer to write
 * \param line Current line in the framebuffer
 * \param color Color to write the buffer with (default: 0x07)
 *
 * \note This function writes a buffer to the framebuffer at the current line
 * and column position with the provided color.
 * \note The line pointer is incremented to the next line to allow for
 * the next write to the framebuffer.
 */
void write_fb(char *fb, const char *buf, size_t *line, u8 color);

/**
 * \brief Clear the framebuffer
 * \param fb Framebuffer to clear
 *
 * \note This function clears the framebuffer by setting each character to a
 * space with the default color.
 */
void clear_fb(char *fb);

/**
 * \brief Initialize the framebuffer
 * \param fb   Framebuffer to initialize
 * \param line Current line in the framebuffer
 *
 * \note This function initializes the framebuffer by clearing it and writing
 * an initialization message.
 */
void init_fb(char *fb, size_t *line);

#endif /* ! GRAPHICS_H */
