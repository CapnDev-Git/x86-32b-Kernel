#include "memory.h"
#include <k/graphics_colors.h>
#include <k/iso9660.h>
#include <k/kstd.h>
#include <stdio.h>
#include <string.h>

#include "atapi.h"
#include "gdt.h"
#include "graphics.h"
#include "idt.h"
#include "keyboard/keyboard.h"
#include "multiboot.h"
#include "serial.h"
#include "timer/timer.h"

#define VGA_ADDRESS 0xb8000

static void halt_CPU(void) {
  for (;;)
    asm volatile("hlt");
}

void k_main(unsigned long magic, multiboot_info_t *info) {
  (void)magic;
  (void)info;

  // Initialize the memory manager
  memory_init(info);

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
  if (init_gdt() != 0) {
    write_fb(fb, "Failed to load GDT", &line, LIGHT_RED);
    halt_CPU();
  }
  write_fb(fb, "GDT loaded", &line, LIGHT_GREEN);
  write_fb(fb, "Protected mode enabled", &line, LIGHT_CYAN);

  // Setup IDT
  write_fb(fb, "Loading IDT...", &line, LIGHT_RED);
  if (init_idt() != 0) {
    write_fb(fb, "Failed to load IDT", &line, LIGHT_RED);
    halt_CPU();
  }
  write_fb(fb, "IDT loaded", &line, LIGHT_GREEN);

  /* --- Setup IRQs --- */
  write_fb(fb, "Loading IRQs...", &line, LIGHT_RED);

  // Setup the timer
  if (init_timer() != 0) {
    write_fb(fb, "Failed to initialize timer", &line, LIGHT_RED);
    halt_CPU();
  }
  write_fb(fb, "Timer initialized", &line, YELLOW);

  // Setup the keyboard
  if (init_keyboard() != 0) {
    write_fb(fb, "Failed to initialize keyboard", &line, LIGHT_RED);
    halt_CPU();
  }
  write_fb(fb, "Keyboard initialized", &line, YELLOW);

  /* Other IRQs... */

  write_fb(fb, "IRQs loaded", &line, LIGHT_GREEN);

  // Halt the CPU
  halt_CPU();
}
