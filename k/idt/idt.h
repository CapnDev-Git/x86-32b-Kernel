#ifndef IDT_H
#define IDT_H

#include <k/types.h>

#define NB_IDT_ENTRIES 256

/**
 * \brief Interrupt registers structure used to store the state of the CPU when
 * an interrupt is triggered.
 * The structure is pushed onto the stack by the CPU when an interrupt is
 * triggered. The structure is used by the interrupt service routine (ISR) to
 * determine the cause of the interrupt and to handle it accordingly.
 * \param cr2 The address that caused the page fault
 * \param ds The data segment selector
 * \param edi, esi, ebp, esp, ebx, edx, ecx, eax The general purpose registers
 * \param int_no The interrupt number
 * \param err_code The error code
 * \param eip The instruction pointer
 * \param cs The code segment selector
 * \param eflags The flags register
 * \param useresp The user stack pointer
 * \param ss The stack segment selector
 * \see isr_handler.c
 */
struct iregs {
  u32 cr2;
  u32 ds;
  u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
  u32 int_no, err_code;
  u32 eip, cs, eflags, useresp, ss;
};

/**
 * \brief Populates the IDT with the default handlers & loads the IDTR strcture
 * into the CPU.
 */
void init_idt(void);

#endif /* ! IDT_H */
