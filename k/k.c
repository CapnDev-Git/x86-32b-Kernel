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
#include <stdio.h>
#include <string.h>

#include "gdt.h"
#include "graphics.h"
#include "idt.h"
#include "keyboard/keyboard.h"
#include "multiboot.h"
#include "serial.h"
#include "timer/timer.h"

#define VGA_ADDRESS 0xb8000

void k_main(unsigned long magic, multiboot_info_t *info) {
  (void)magic;
  (void)info;

  size_t line = 0;
  char *fb = (void *)VGA_ADDRESS; // Framebuffer address

  /* --- Setup Framebuffer --- */
  // Initialize the framebuffer
  init_fb(fb, &line);

  /* --- Setup Serial --- */
  // Initialize the serial port at COM1
  serial_init();
  write_fb(fb, "Serial port initialized", &line, YELLOW);

  // Write a test message to the serial port
  printf("Hello Serial Port!\n");

  /* --- Setup Tables --- */
  // Setup GDT
  write_fb(fb, "Loading GDT...", &line, LIGHT_RED);
  init_gdt();
  write_fb(fb, "GDT loaded", &line, LIGHT_GREEN);
  write_fb(fb, "Protected mode enabled", &line, LIGHT_CYAN);

  // Setup IDT
  write_fb(fb, "Loading IDT...", &line, LIGHT_RED);
  init_idt();
  write_fb(fb, "IDT loaded", &line, LIGHT_GREEN);

  /* --- Setup IRQs --- */
  write_fb(fb, "Loading IRQs...", &line, LIGHT_RED);

  // Setup the timer
  init_timer();
  write_fb(fb, "Timer initialized", &line, LIGHT_CYAN);

  // Setup the keyboard
  init_keyboard();
  write_fb(fb, "Keyboard initialized", &line, LIGHT_CYAN);

  // Trigger a divide-by-zero error
  // __asm__ volatile("int $0x00");
  // printf("Shouldn't print\n");

  // Halt the CPU
  for (;;)
    asm volatile("hlt");
}
