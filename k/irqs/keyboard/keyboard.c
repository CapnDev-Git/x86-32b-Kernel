#include "keyboard.h"

#include <k/types.h> // u8, u32, u64
#include <stdbool.h> // bool
#include <stdio.h>   // printf

#include "../../io.h" // outb
#include "irq.h"      // irq_install_handler

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_EXTRACT_SCANCODE 0x7F
#define KEYBOARD_RELEASED_MASK 0x80

#define KEYCODE_LSHIFT 0x2A
#define KEYCODE_RSHIFT 0x36
#define KEYCODE_CAPSLOCK 0x3A
#define KEYCODE_LCTRL 0x1D
#define KEYCODE_BACKSPACE 0x0E
#define KEYCODE_ENTER 0x1C
#define KEYCODE_ESC 0x01
#define KEYCODE_LALT 0x38
#define KEYCODE_SPACE 0x39
#define KEYCODE_TAB 0x0F

#define ASCII_LOWER_A 0x61
#define ASCII_LOWER_Z 0x7A
#define ASCII_UPPERCASE_OFFSET 0x20

bool simple_caps_on;
bool caps_lock_on;

static char key_mappings_qwerty[] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
    [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
    [0x15] = 'y', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
    [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l', [0x2C] = 'z',
    [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v', [0x30] = 'b', [0x31] = 'n',
    [0x32] = 'm'};

static char *key_mappings_special[] = {[KEYCODE_LSHIFT] = "LSHIFT",
                                       [KEYCODE_RSHIFT] = "RSHIFT",
                                       [KEYCODE_CAPSLOCK] = "CAPSLOCK",
                                       [KEYCODE_LCTRL] = "LCTRL",
                                       [KEYCODE_BACKSPACE] = "BACKSPACE",
                                       [KEYCODE_ENTER] = "ENTER",
                                       [KEYCODE_ESC] = "ESC",
                                       [KEYCODE_LALT] = "LALT",
                                       [KEYCODE_SPACE] = "SPACE",
                                       [KEYCODE_TAB] = "TAB"};

static char get_printable_char(u8 scancode) {
  char c = key_mappings_qwerty[scancode];
  if (c >= ASCII_LOWER_A && c <= ASCII_LOWER_Z)
    if (simple_caps_on || caps_lock_on)
      return c - ASCII_UPPERCASE_OFFSET;
  return c;
}

static void on_irq1(struct iregs *regs) {
  (void)regs;

  // Read the scan code from the keyboard (get the pressed key)
  u8 scancode = inb(KEYBOARD_DATA_PORT) & KEYBOARD_EXTRACT_SCANCODE;
  u8 pressed = inb(KEYBOARD_DATA_PORT) & KEYBOARD_RELEASED_MASK;

  // Activate the simple cap
  if (scancode == KEYCODE_LSHIFT || scancode == KEYCODE_RSHIFT) {
    simple_caps_on = !pressed;
    return;
  } else if (scancode == KEYCODE_CAPSLOCK) {
    if (!pressed)
      caps_lock_on = !caps_lock_on;
    return;
  }

  // Print the pressed key to the console (with or without caps)
  if (pressed) {
    if (key_mappings_special[scancode] != NULL)
      printf("[%s]\n", key_mappings_special[scancode]);
    else
      printf("'%c'\n", get_printable_char(scancode));
  }
}

int init_keyboard(void) {
  // Set the caps & caps lock state to false (off)
  simple_caps_on = false;
  caps_lock_on = false;

  // Install the IRQ handler for the keyboard
  if (irq_install_handler(1, &on_irq1) != 0) {
    printf("Failed to install keyboard handler\n");
    return -1;
  }
  printf("Keyboard initialized\n");

  return 0; // Success
}
