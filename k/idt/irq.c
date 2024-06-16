#include "irq.h"

#include "../io.h"

// IRQ handlers array
static void *irq_routines[NB_IRQS] = {0};

/**
 * \brief Adds a handler for the given IRQ
 * \param irq The IRQ number
 * \param handler The handler function
 */
static void irq_install_handler(int irq, void (*handler)(struct iregs *r)) {
  irq_routines[irq] = handler;
}

/**
 * \brief Removes the handler for the given IRQ
 * \param irq The IRQ number
 */
static void irq_uninstall_handler(int irq) { irq_routines[irq] = 0; }

void irq_handler(struct iregs *regs) {
  // Get the handler from the IRQ routine
  void (*handler)(struct iregs * r);
  handler = irq_routines[regs->int_no - NB_DEFINED_INTERRUPTS];

  // Call the handler if there is one associated with the IRQ
  if (handler)
    handler(regs);

  // Send an EOI (end of interrupt) signal to the PICs.
  if (regs->int_no >= 40)
    outb(0xA0, 0x20);

  // Send an EOI to the master PIC
  outb(0x20, 0x20);
}
