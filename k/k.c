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
#include <k/graphics_colors.h>
#include <k/kstd.h>
#include <string.h>

#include "gdt.h"
#include "graphics.h"
#include "multiboot.h"
#include "serial.h"

#define VGA_ADDRESS 0xb8000

void k_main(unsigned long magic, multiboot_info_t *info) {
  (void)magic;
  (void)info;

  size_t line = 0;
  char *fb = (void *)VGA_ADDRESS; // Framebuffer address

  // Initialize the framebuffer
  init_fb(fb, &line);

  // Initialize the serial port at COM1
  serial_init();
  write_fb(fb, "Serial port initialized", &line, LIGHT_GREEN);

  // Write a test message to the serial port
  write("Hello Serial Port!\r\n", 20);

  // Setup GDT
  init_gdt();
  write_fb(fb, "GDT loaded", &line, YELLOW);
  write_fb(fb, "Protected mode enabled", &line, LIGHT_CYAN);

  // Halt the CPU
  for (;;)
    asm volatile("hlt");
}
