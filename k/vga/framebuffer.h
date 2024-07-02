#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <k/types.h>
#include <stddef.h>

/**
 * \brief Set the video mode
 * \param mode The video mode to set
 * \return 0 on success, -1 on failure
 * \note The function sets the video mode to either text or graphic mode, which
 * are defined in the vga.h header file. This allows for switching between text
 * and graphic modes for the framebuffer.
 */
int setvideo(int mode);

/**
 * \brief Swap the front buffer
 * \param buffer The buffer to swap to the front
 * \note The function swaps the front buffer with the provided buffer. This
 * allows for double buffering in graphic mode to prevent flickering when
 * drawing to the screen.
 */
void swap_frontbuffer(const void *buffer);

/**
 * \brief Write a buffer to the framebuffer
 * \param fb   Framebuffer to write to
 * \param buf  Buffer to write
 * \param line Current line in the framebuffer
 * \param color Color to write the buffer with (default: 0x07)
 *
 * \note This function writes a buffer to the framebuffer at the current
 * line and column position with the provided color. \note The line
 * pointer is incremented to the next line to allow for the next write to
 * the framebuffer.
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

#endif /* ! FRAMEBUFFER_H */
