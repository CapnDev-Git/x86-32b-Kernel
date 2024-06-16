#include "keyboard.h"

#include <k/types.h> // u8, u32, u64
#include <stdbool.h> // bool
#include <stdio.h>   // printf

#include "../../io.h" // outb
#include "irq.h"      // irq_install_handler

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

#define KEYCODE_LSHIFT 0x2A
#define KEYCODE_RSHIFT 0x36
#define KEYCODE_CAPSLOCK 0x3A

bool simple_caps_on;
bool caps_lock_on;

static char key_mappings_qwerty[] = {
    [0x01] = 0x1B, [0x02] = '1',  [0x03] = '2',  [0x04] = '3',  [0x05] = '4',
    [0x06] = '5',  [0x07] = '6',  [0x08] = '7',  [0x09] = '8',  [0x0A] = '9',
    [0x0B] = '0',  [0x0E] = '\b', [0x0F] = '\t', [0x10] = 'q',  [0x11] = 'w',
    [0x12] = 'e',  [0x13] = 'r',  [0x14] = 't',  [0x15] = 'y',  [0x16] = 'u',
    [0x17] = 'i',  [0x18] = 'o',  [0x19] = 'p',  [0x1C] = '\n', [0x1E] = 'a',
    [0x1F] = 's',  [0x20] = 'd',  [0x21] = 'f',  [0x22] = 'g',  [0x23] = 'h',
    [0x24] = 'j',  [0x25] = 'k',  [0x26] = 'l',  [0x2C] = 'z',  [0x2D] = 'x',
    [0x2E] = 'c',  [0x2F] = 'v',  [0x30] = 'b',  [0x31] = 'n',  [0x32] = 'm'};

static void on_irq1(struct iregs *regs) {
  (void)regs;

  // Read the scan code from the keyboard (get the pressed key)
  u8 scancode = inb(KEYBOARD_DATA_PORT) & 0x7F;
  u8 pressed = inb(KEYBOARD_DATA_PORT) & 0x80;
  char key = key_mappings_qwerty[scancode];

  // Activate the simple caps
  if (scancode == KEYCODE_LSHIFT || scancode == KEYCODE_RSHIFT) {
    simple_caps_on = !pressed;
    return;
  } else if (scancode == KEYCODE_CAPSLOCK) {
    if (!pressed)
      caps_lock_on = !caps_lock_on;
    return;
  }

  // Check if the key is pressed
  if (pressed)
    // Print the key to the console (with or without caps)
    printf("'%c'\n", (simple_caps_on || caps_lock_on) ? key - 32 : key);
}

void init_keyboard(void) {
  // Set the caps & caps lock state to false (off)
  simple_caps_on = false;
  caps_lock_on = false;

  // Install the IRQ handler for the keyboard
  irq_install_handler(1, &on_irq1);
  printf("Keyboard initialized\n");

  //   // Enable the keyboard
  //   outb(KEYBOARD_STATUS_PORT, 0xAE);
  //   outb(KEYBOARD_STATUS_PORT, 0x20);

  //   // Read the current state of the keyboard
  //   u8 status = (inb(KEYBOARD_DATA_PORT) | 1) & ~0x10;
  //   outb(KEYBOARD_STATUS_PORT, 0x60);
  //   outb(KEYBOARD_DATA_PORT, status);
  //   outb(KEYBOARD_DATA_PORT, 0xF4);

  //   // Log the keyboard status
  //   printf("Keyboard status: %d\n", status);

  //   // Enable the keyboard IRQ
  //   irq_enable(1);

  //   // Log the keyboard IRQ enabled
  //   printf("Keyboard IRQ enabled\n");

  //   // Log the keyboard initialized
  //   printf("Keyboard initialized\n");
}
