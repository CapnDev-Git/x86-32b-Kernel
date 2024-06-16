#ifndef ISR_H
#define ISR_H

#include "idt.h"

#define NB_ISRS NB_IDT_ENTRIES

/**
 * \brief Interrupt Service Routine (ISR) handler
 * \param regs The interrupt registers structure
 */
void isr_handler(struct iregs *regs);

// ISR handlers (declaration for ASM linkage)
extern void isr0(void);  // Division by zero
extern void isr1(void);  // Debug exception
extern void isr2(void);  // Non maskable interrupt
extern void isr3(void);  // Breakpoint exception
extern void isr4(void);  // Into detected overflow
extern void isr5(void);  // Out of bounds
extern void isr6(void);  // Invalid opcode
extern void isr7(void);  // No coprocessor
extern void isr8(void);  // Double fault
extern void isr9(void);  // Coprocessor segment
extern void isr10(void); // Bad TSS
extern void isr11(void); // Segment not present
extern void isr12(void); // Stack fault
extern void isr13(void); // General protection
extern void isr14(void); // Page fault
extern void isr15(void); // Unknown interrupt
extern void isr16(void); // Coprocessor fault
extern void isr17(void); // Alignment check
extern void isr18(void); // Machine check
extern void isr19(void); // Reserved
extern void isr20(void); // Reserved
extern void isr21(void); // Reserved
extern void isr22(void); // Reserved
extern void isr23(void); // Reserved
extern void isr24(void); // Reserved
extern void isr25(void); // Reserved
extern void isr26(void); // Reserved
extern void isr27(void); // Reserved
extern void isr28(void); // Reserved
extern void isr29(void); // Reserved
extern void isr30(void); // Reserved
extern void isr31(void); // Reserved
// -- 32 to 127 are reserved
extern void isr128(void); // System call
extern void isr177(void); // System call

#endif /* ! ISR_H */
