#ifndef IRQ_H
#define IRQ_H

#define NB_IRQS 16

#include "idt.h"

/**
 * \brief Triggers the corresponding IRQ handler from the interrupt registers
 * structure provided
 * \param regs The interrupt registers structure
 */
void irq_handler(struct iregs *r);

/**
 * \brief Adds a handler for the given IRQ
 * \param irq The IRQ number
 * \param handler The handler function
 */
void irq_install_handler(int irq, void (*handler)(struct iregs *r));

/**
 * \brief Removes the handler for the given IRQ
 * \param irq The IRQ number
 */
void irq_uninstall_handler(int irq);

// IRQ handlers (declaration for ASM linkage)
extern void irq0(void);  // Timer
extern void irq1(void);  // Keyboard
extern void irq2(void);  // Cascade
extern void irq3(void);  // COM2
extern void irq4(void);  // COM1
extern void irq5(void);  // LPT2
extern void irq6(void);  // Floppy
extern void irq7(void);  // LPT1
extern void irq8(void);  // CMOS
extern void irq9(void);  // Free
extern void irq10(void); // Free
extern void irq11(void); // Free
extern void irq12(void); // Mouse
extern void irq13(void); // FPU
extern void irq14(void); // Primary ATA
extern void irq15(void); // Secondary ATA

#endif /* ! IRQ_H */
