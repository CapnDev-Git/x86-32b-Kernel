#include "timer.h"

#include <k/types.h> // u8, u32, u64
#include <stdio.h>   // printf

#include "../../io.h" // outb
#include "irq.h"      // irq_install_handler

#define TIMER_ZERO 0
#define CLOCK_TICK_RATE 1193182
#define INTERRUPT_FREQ_RATE ((u32)100)

#define COUNTER_0 0x40
#define CONTROL_REGISTER 0x43
#define PIT_CONFIG 0x34
#define DIVISOR_LOW(divisor) ((u8)(divisor & 0xFF))
#define DIVISOR_HIGH(divisor) ((u8)((divisor >> 8) & 0xFF))

u32 ticks = TIMER_ZERO;
u32 freq = INTERRUPT_FREQ_RATE;

/**
 * \brief Timer interrupt handler (IRQ0)
 * \param regs The interrupt registers (unused but required for IRQ handler)
 */
static void on_irq0(struct iregs *regs) {
  (void)regs;

  // Increment the tick count
  ticks++;

  // Log the tick count every second
  if (ticks % INTERRUPT_FREQ_RATE == 0) {
    // printf("Tick: %lu\n", ticks / 100);
  }
}

int init_timer(void) {
  // Reset the tick count
  ticks = TIMER_ZERO;
  printf("Initializing timer\n");

  // Install the IRQ handler for the timer
  if (irq_install_handler(0, &on_irq0) != 0) {
    printf("Failed to install timer handler\n");
    return -1;
  }

  // Set the timer frequency
  u32 divisor = CLOCK_TICK_RATE / freq;

  // Send the command byte to the PIT
  outb(CONTROL_REGISTER, PIT_CONFIG);     // counter 0, LB/HB, mode 2, binary
  outb(COUNTER_0, DIVISOR_LOW(divisor));  // low byte of divisor to PIT.c0
  outb(COUNTER_0, DIVISOR_HIGH(divisor)); // high byte of divisor to PIT.c0
  printf("Timer initialized on Mode 2, divisor: %d\n", divisor);

  return 0; // Success
}
