/*
 * Copyright (c) LSE
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY LSE AS IS AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL LSE BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <k/kstd.h>

#include "multiboot.h"
#include "serial.h"

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

void k_main(unsigned long magic, multiboot_info_t *info) {
  (void)magic;
  (void)info;

  size_t line = 0;
  char *fb = (void *)0xb8000; // Framebuffer address

  // Initialize the kernel
  init_framebuffer(fb, &line);

  // Initialize the serial port at COM1
  serial_init();
  write_framebuffer(fb, "Serial Port Initialized!", &line);

  // Write a test message to the serial port
  write("Hello Serial Port!\r\n", 19);

  // Write a test message to the framebuffer
  write_framebuffer(fb, "Hello Framebuffer!", &line);
}
