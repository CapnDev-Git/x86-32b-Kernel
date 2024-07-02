#include "memory.h"
#include <k/graphics_colors.h>
#include <k/iso9660.h>
#include <k/kstd.h>
#include <stdio.h>
#include <string.h>

#include "atapi.h"
#include "framebuffer.h"
#include "gdt.h"
#include "idt.h"
#include "iso_filesystem.h"
#include "iso_helpers.h"
#include "keyboard/keyboard.h"
#include "multiboot.h"
#include "serial.h"
#include "syscalls/syscalls.h"
#include "timer/timer.h"
#include "vga.h"

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
  write_fb(fb, "Serial port initialized", &line, FRAMEBUFFER_YELLOW);

  // Write a test message to the serial port
  printf("Hello Serial Port!\n");

  /* --- Setup Tables --- */
  // Setup GDT
  write_fb(fb, "Loading GDT...", &line, FRAMEBUFFER_LIGHT_RED);
  if (init_gdt() != 0) {
    write_fb(fb, "Failed to load GDT", &line, FRAMEBUFFER_LIGHT_RED);
    halt_CPU();
  }
  write_fb(fb, "GDT loaded", &line, FRAMEBUFFER_LIGHT_GREEN);
  write_fb(fb, "Protected mode enabled", &line, FRAMEBUFFER_LIGHT_CYAN);

  // Setup IDT
  write_fb(fb, "Loading IDT...", &line, FRAMEBUFFER_LIGHT_RED);
  if (init_idt() != 0) {
    write_fb(fb, "Failed to load IDT", &line, FRAMEBUFFER_LIGHT_RED);
    halt_CPU();
  }
  write_fb(fb, "IDT loaded", &line, FRAMEBUFFER_LIGHT_GREEN);

  /* --- Setup ISRs --- */
  write_fb(fb, "Loading ISRs...", &line, FRAMEBUFFER_LIGHT_RED);

  // Setup the system calls
  if (init_syscalls() != 0) {
    write_fb(fb, "Failed to initialize syscalls", &line, FRAMEBUFFER_LIGHT_RED);
    halt_CPU();
  }
  write_fb(fb, "Syscalls initialized", &line, FRAMEBUFFER_YELLOW);

  write_fb(fb, "ISRs loaded", &line, FRAMEBUFFER_LIGHT_GREEN);

  /* --- Setup IRQs --- */
  write_fb(fb, "Loading IRQs...", &line, FRAMEBUFFER_LIGHT_RED);

  // Setup the timer
  if (init_timer() != 0) {
    write_fb(fb, "Failed to initialize timer", &line, FRAMEBUFFER_LIGHT_RED);
    halt_CPU();
  }
  write_fb(fb, "Timer initialized", &line, FRAMEBUFFER_YELLOW);

  // Setup the keyboard
  if (init_keyboard() != 0) {
    write_fb(fb, "Failed to initialize keyboard", &line, FRAMEBUFFER_LIGHT_RED);
    halt_CPU();
  }
  write_fb(fb, "Keyboard initialized", &line, FRAMEBUFFER_YELLOW);

  /* Other IRQs... */

  write_fb(fb, "IRQs loaded", &line, FRAMEBUFFER_LIGHT_GREEN);

  /* --- Setup ATAPI --- */
  write_fb(fb, "Discovering ATAPI drive...", &line, FRAMEBUFFER_LIGHT_RED);
  if (discover_atapi_drive() != 0) {
    write_fb(fb, "ATAPI drive not found", &line, FRAMEBUFFER_LIGHT_RED);
    halt_CPU();
  }
  write_fb(fb, "ATAPI drive found", &line, FRAMEBUFFER_LIGHT_GREEN);

  write_fb(fb, "Setup finished!", &line, FRAMEBUFFER_LIGHT_CYAN);

  // char *chiche_path = "/usr/pong/res/chiche.bmp";
  // char *yaka_path = "/usr/yakanoid/res/yaka2009.bmp";
  // char *pagnoux_path = "/usr/pong/res/pagnoux.bmp";
  // switch_graphic();
  // draw_begin();
  // struct image *yaka = load_image(pagnoux_path);
  // draw_image(yaka, 117, 25);
  // struct image *chiche = load_image(chiche_path);
  // draw_image(chiche, -15, 10);
  // draw_text(" SharkKernel ", 110, 50, WHITE, 0);
  // draw_text("powered by Capn", 100, 60, WHITE, 0);
  // draw_text("Electif Kernel - 2023-2024", 50, 188, RED, 0);
  // draw_end();

  // memory_dump_leaks();

  // Halt the CPU
  halt_CPU();
}
