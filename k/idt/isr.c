#include "isr.h"

#include <stdio.h> // printf

// Exception messages array
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

void isr_handler(struct iregs *regs) {
  printf("Received interrupt: %d\n", regs->int_no);

  // If the interrupt number is less than 32, it's an exception (fault)
  if (regs->int_no < 32) {
    printf("-> Exception: %s\n", exception_messages[regs->int_no]);
    printf("-> Error code: %d\n", regs->err_code);
    printf("Processor halted!\n");
    for (;;)
      ;
  }
}
