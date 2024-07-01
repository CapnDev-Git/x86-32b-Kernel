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
#include "iso_filesystem.h"
#include "iso_helpers.h"
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

  /* --- Setup ATAPI --- */

  write_fb(fb, "Discovering ATAPI drive...", &line, LIGHT_RED);
  if (discover_atapi_drive() != 0) {
    write_fb(fb, "ATAPI drive not found", &line, LIGHT_RED);
    halt_CPU();
  }
  write_fb(fb, "ATAPI drive found", &line, LIGHT_GREEN);

  /* --- Setup ISO9660 --- */

  write_fb(fb, "Loading ISO9660 filesystem...", &line, LIGHT_RED);
  struct iso_filesystem *filesystem = init_filesystem();
  if (!filesystem) {
    write_fb(fb, "Failed to load ISO9660 filesystem", &line, LIGHT_RED);
    halt_CPU();
  }
  write_fb(fb, "ISO9660 filesystem loaded", &line, LIGHT_GREEN);

  // Get the file buffer from the path
  char *path = "/USR/PONG/RES/BALL.BMP";
  printf("Fetching file at path: %s\n", path);

  write_fb(fb, "Fetching file...", &line, LIGHT_RED);
  void *file_entry_buffer = get_file_from_path(filesystem, path);
  if (!file_entry_buffer) {
    printf("Failed to fetch file\n");
    write_fb(fb, "Failed to fetch file entry", &line, LIGHT_RED);
    halt_CPU();
  }
  write_fb(fb, "File entry fetched", &line, LIGHT_GREEN);

  // Free all buffers & path ids
  memory_release(file_entry_buffer);
  free_filesystem(filesystem);

  // memory_dump_leaks(); // TO FIX

  // Halt the CPU
  halt_CPU();
}
