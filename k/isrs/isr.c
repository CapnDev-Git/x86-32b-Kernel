#include "isr.h"

#include <stdio.h> // printf

/**
 * \brief Array of exception messages for each ISR
 */
static char *exception_messages[NB_ISRS] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment not present",
    "Stack fault",
    "General protection fault",
    "Page fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Fault",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    [128] = "System call",
    [177] = "System call",
};

/**
 * \brief Array of ISR handlers
 */
static void *isr_routines[NB_ISRS] = {0};

int isr_install_handler(int isr, void (*handler)(struct iregs *r)) {
  printf("Installing ISR handler %d\n", isr);
  isr_routines[isr] = handler;
  return 0;
}

void isr_uninstall_handler(int isr) {
  // Remove the handler for the given ISR
  isr_routines[isr] = 0;
}

void isr_handler(struct iregs *r) {
  // Log the interrupt number
  printf("Received interrupt: %d\n", r->int_no);

  // If the interrupt number is less than 32, it's an exception (fault)
  // and we need to halt the CPU (?)
  if (r->int_no < NB_DEFINED_INTERRUPTS) {
    // Log the exception
    printf("-> Exception: %s\n", exception_messages[r->int_no]);
    printf("-> Error code: %d\n", r->err_code);
    printf("Processor halted!\n");

    // Halt the CPU
    for (;;)
      ;
  }

  // Get the handler from the ISR routine
  void (*handler)(struct iregs * r);
  handler = isr_routines[r->int_no];

  // Call the handler if there is one associated with the IRQ
  if (handler)
    handler(r);
}
