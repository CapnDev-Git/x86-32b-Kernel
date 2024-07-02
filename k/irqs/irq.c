#include "irq.h"

#include "../io.h" // outb
#include <stdio.h> // printf

#define SLAVE_FLAG 0x28
#define PIC_MASTER_A 0x20
#define PIC_SLAVE_A 0xA0
#define EOI 0x20

/**
 * \brief Array of IRQ handlers
 */
static void *irq_routines[NB_IRQS] = {0};

int irq_install_handler(int irq, void (*handler)(struct iregs *r)) {
  printf("Installing IRQ handler %d\n", irq);
  irq_routines[irq] = handler;
  return 0;
}

void irq_uninstall_handler(int irq) {
  // Remove the handler for the given IRQ
  irq_routines[irq] = 0;
}

void irq_handler(struct iregs *r) {
  // Get the handler from the IRQ routine
  void (*handler)(struct iregs * r);
  handler = irq_routines[r->int_no - NB_DEFINED_INTERRUPTS];

  // Call the handler if there is one associated with the IRQ
  if (handler)
    handler(r);

  // Send an EOI (end of interrupt) signal to the PICs.
  if (r->int_no >= SLAVE_FLAG)
    outb(PIC_SLAVE_A, EOI);

  // Send an EOI to the master PIC
  outb(PIC_MASTER_A, EOI);
}
