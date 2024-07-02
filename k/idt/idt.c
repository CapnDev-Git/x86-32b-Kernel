#include "idt.h" // struct idt_gate, struct idtr, idt_flush symbol

#include <k/types.h> // u8, u16, u32
#include <stdio.h>   // printf
#include <string.h>  // memset

#include "../io.h" // outb
#include "irq.h"   // IRQ handlers
#include "isr.h"   // ISR handlers

// IDT-related
#define INTERRUPT_GATE_LOW(base) (base & 0xFFFF)
#define INTERRUPT_GATE_HIGH(base) ((base >> 16) & 0xFFFF)
#define INTERRUPT_GATE_FLAGS(flags) (flags | 0x60)
#define INTERRUPT_SEL 0x08
#define INTERRUPT_FLAGS 0x8E

// Programmable Interrupt Controller (PIC) ports and values
#define PIC_MASTER_A 0x20
#define PIC_MASTER_B 0x21
#define PIC_SLAVE_A 0xA0
#define PIC_SLAVE_B 0xA1
#define A_ICW1 0x11
#define MASTER_BASE_OFFSET 0x20
#define SLAVE_BASE_OFFSET 0x28
#define IRQ_MASTER 0x04
#define IRQ_SLAVE 0x02
#define OPERATION_MODE 0x01
#define MASK_IRQ 0xFF
#define UNMASK_IRQ 0x00

/**
 * \brief IDT gate structure
 * The IDT gate structure is used to define the properties of an interrupt
 * descriptor in the Interrupt Descriptor Table (IDT). The IDT is a structure
 * used by the CPU to store interrupt descriptors. The IDT gate structure is
 * used to define the properties of an interrupt descriptor in the IDT. The IDT
 * gate structure is used by the CPU to determine the properties of an interrupt
 * handler when an interrupt is triggered.
 * \param base_low The low 16 bits of the base address of the interrupt handler
 * \param sel The segment selector
 * \param zero This field is always zero
 * \param flags The flags for the IDT entry
 * \param base_high The high 16 bits of the base address of the interrupt
 * handler
 */
struct idt_gate {
  u16 base_low;
  u16 sel;
  u8 zero;
  u8 flags;
  u16 base_high;
} __attribute__((packed));

/**
 * \brief IDTR structure
 * The IDTR structure is used to define the properties of the Interrupt
 * Descriptor Table Register (IDTR). The IDTR structure is used to define the
 * properties of the IDT in memory.
 * \param limit The size of the IDT
 * \param base The base address of the IDT
 */
struct idtr {
  u16 limit;
  u32 base;
} __attribute__((packed));

/**
 * \brief Sets up the IDT table in memory ASM function in
 * idt_setup.S
 * \param idt_ptr The address of the IDTR structure
 */
extern void idt_flush(u32 idt_ptr);

struct idt_gate idt_entries[NB_IDT_ENTRIES];
struct idtr idtr;

/**
 * \brief Sets an IDT gate
 * \param num The IDT entry number
 * \param base The base address of the interrupt handler
 * \param sel The segment selector
 * \param flags The flags for the IDT entry
 */
static void set_idt_gate(u8 num, u32 base, u16 sel, u8 flags) {
  idt_entries[num].base_low = INTERRUPT_GATE_LOW(base);
  idt_entries[num].base_high = INTERRUPT_GATE_HIGH(base);
  idt_entries[num].sel = sel;
  idt_entries[num].zero = 0;
  idt_entries[num].flags = INTERRUPT_GATE_FLAGS(flags);
}

/**
 * \brief Sets up all the IDT gates
 */
static void set_idt_gates(void) {
  // ISRs
  set_idt_gate(0, (u32)isr0, INTERRUPT_SEL, INTERRUPT_FLAGS); // Divide by zero
  set_idt_gate(1, (u32)isr1, INTERRUPT_SEL, INTERRUPT_FLAGS); // Debug
  set_idt_gate(2, (u32)isr2, INTERRUPT_SEL, INTERRUPT_FLAGS); // NMI
  set_idt_gate(3, (u32)isr3, INTERRUPT_SEL, INTERRUPT_FLAGS); // Breakpoint
  set_idt_gate(4, (u32)isr4, INTERRUPT_SEL, INTERRUPT_FLAGS); // Overflow
  set_idt_gate(5, (u32)isr5, INTERRUPT_SEL,
               INTERRUPT_FLAGS); // BOUND Range Exceeded
  set_idt_gate(6, (u32)isr6, INTERRUPT_SEL, INTERRUPT_FLAGS); // Invalid Opcode
  set_idt_gate(7, (u32)isr7, INTERRUPT_SEL,
               INTERRUPT_FLAGS); // Device Not Available
  set_idt_gate(8, (u32)isr8, INTERRUPT_SEL, INTERRUPT_FLAGS); // Double Fault
  set_idt_gate(9, (u32)isr9, INTERRUPT_SEL,
               INTERRUPT_FLAGS); // Coprocessor Segment Overrun
  set_idt_gate(10, (u32)isr10, INTERRUPT_SEL, INTERRUPT_FLAGS); // Invalid TSS
  set_idt_gate(11, (u32)isr11, INTERRUPT_SEL,
               INTERRUPT_FLAGS); // Segment Not Present
  set_idt_gate(12, (u32)isr12, INTERRUPT_SEL,
               INTERRUPT_FLAGS); // Stack-Segment Fault
  set_idt_gate(13, (u32)isr13, INTERRUPT_SEL,
               INTERRUPT_FLAGS); // General Protection Fault
  set_idt_gate(14, (u32)isr14, INTERRUPT_SEL, INTERRUPT_FLAGS); // Page Fault
  set_idt_gate(15, (u32)isr15, INTERRUPT_SEL,
               INTERRUPT_FLAGS); // Unknown Interrupt
  set_idt_gate(16, (u32)isr16, INTERRUPT_SEL,
               INTERRUPT_FLAGS); // Coprocessor Fault
  set_idt_gate(17, (u32)isr17, INTERRUPT_SEL,
               INTERRUPT_FLAGS); // Alignment Check
  set_idt_gate(18, (u32)isr18, INTERRUPT_SEL, INTERRUPT_FLAGS); // Machine Check
  set_idt_gate(19, (u32)isr19, INTERRUPT_SEL,
               INTERRUPT_FLAGS); // SIMD Floating-Point Exception
  set_idt_gate(20, (u32)isr20, INTERRUPT_SEL,
               INTERRUPT_FLAGS); // Virtualization Exception
  set_idt_gate(21, (u32)isr21, INTERRUPT_SEL,
               INTERRUPT_FLAGS); // Control Protection Exception
  set_idt_gate(22, (u32)isr22, INTERRUPT_SEL, INTERRUPT_FLAGS); // Reserved
  set_idt_gate(23, (u32)isr23, INTERRUPT_SEL, INTERRUPT_FLAGS); // Reserved
  set_idt_gate(24, (u32)isr24, INTERRUPT_SEL, INTERRUPT_FLAGS); // Reserved
  set_idt_gate(25, (u32)isr25, INTERRUPT_SEL, INTERRUPT_FLAGS); // Reserved
  set_idt_gate(26, (u32)isr26, INTERRUPT_SEL, INTERRUPT_FLAGS); // Reserved
  set_idt_gate(27, (u32)isr27, INTERRUPT_SEL, INTERRUPT_FLAGS); // Reserved
  set_idt_gate(28, (u32)isr28, INTERRUPT_SEL, INTERRUPT_FLAGS); // Reserved
  set_idt_gate(29, (u32)isr29, INTERRUPT_SEL, INTERRUPT_FLAGS); // Reserved
  set_idt_gate(30, (u32)isr30, INTERRUPT_SEL, INTERRUPT_FLAGS); // Reserved
  set_idt_gate(31, (u32)isr31, INTERRUPT_SEL, INTERRUPT_FLAGS); // Reserved

  // IRQs
  set_idt_gate(32, (u32)irq0, INTERRUPT_SEL, INTERRUPT_FLAGS);  // Timer
  set_idt_gate(33, (u32)irq1, INTERRUPT_SEL, INTERRUPT_FLAGS);  // Keyboard
  set_idt_gate(34, (u32)irq2, INTERRUPT_SEL, INTERRUPT_FLAGS);  // Cascade
  set_idt_gate(35, (u32)irq3, INTERRUPT_SEL, INTERRUPT_FLAGS);  // COM2
  set_idt_gate(36, (u32)irq4, INTERRUPT_SEL, INTERRUPT_FLAGS);  // COM1
  set_idt_gate(37, (u32)irq5, INTERRUPT_SEL, INTERRUPT_FLAGS);  // LPT2
  set_idt_gate(38, (u32)irq6, INTERRUPT_SEL, INTERRUPT_FLAGS);  // Floppy
  set_idt_gate(39, (u32)irq7, INTERRUPT_SEL, INTERRUPT_FLAGS);  // LPT1
  set_idt_gate(40, (u32)irq8, INTERRUPT_SEL, INTERRUPT_FLAGS);  // CMOS
  set_idt_gate(41, (u32)irq9, INTERRUPT_SEL, INTERRUPT_FLAGS);  // Free
  set_idt_gate(42, (u32)irq10, INTERRUPT_SEL, INTERRUPT_FLAGS); // Free
  set_idt_gate(43, (u32)irq11, INTERRUPT_SEL, INTERRUPT_FLAGS); // Free
  set_idt_gate(44, (u32)irq12, INTERRUPT_SEL, INTERRUPT_FLAGS); // Mouse
  set_idt_gate(45, (u32)irq13, INTERRUPT_SEL, INTERRUPT_FLAGS); // FPU
  set_idt_gate(46, (u32)irq14, INTERRUPT_SEL, INTERRUPT_FLAGS); // Primary ATA
  set_idt_gate(47, (u32)irq15, INTERRUPT_SEL, INTERRUPT_FLAGS); // Secondary ATA

  // System calls
  set_idt_gate(128, (u32)isr128, INTERRUPT_SEL,
               INTERRUPT_FLAGS); // Syscall (0x80)
}

/**
 * \brief Initializes the Programmable Interrupt Controller (PIC)
 */
static void init_PIC(void) {
  // Start the both master and slave PICs (ICW1)
  outb(PIC_MASTER_A, A_ICW1); // ICW4 present, edge triggered, cascade mode
  outb(PIC_SLAVE_A, A_ICW1);  // ICW4 present, edge triggered, cascade mode

  // Set the PIC interrupt offsets (ICW2)
  outb(PIC_MASTER_B, MASTER_BASE_OFFSET); // Vector base address for master
  outb(PIC_SLAVE_B, SLAVE_BASE_OFFSET);   // Vector base address for slave

  // Set the PIC cascade identity (ICW3)
  outb(PIC_MASTER_B, IRQ_MASTER); // Inform Master that slave PIC ais at IRQ2
  outb(PIC_SLAVE_B, IRQ_SLAVE);   // Tell Slave PIC its cascade identity

  // Set the PIC mode to 8086 (ICW4)
  outb(PIC_MASTER_B, OPERATION_MODE); // normal, no buffering, not special
  outb(PIC_SLAVE_B, OPERATION_MODE);  // normal, no buffering, not special

  // Mask all IRQs
  outb(PIC_MASTER_B, UNMASK_IRQ); // Master PIC is cascaded to IRQ2
  outb(PIC_SLAVE_B, UNMASK_IRQ);  // Slave PIC is cascaded to IRQ2
}

int init_idt(void) {
  // Set the IDTR properties & clear the IDT
  idtr.limit = (sizeof(struct idt_gate) * NB_IDT_ENTRIES) - 1;
  idtr.base = (u32)&idt_entries;
  memset(&idt_entries, 0, sizeof(struct idt_gate) * NB_IDT_ENTRIES);
  printf("IDTR structure setup\n");

  // Initialize the PIC
  init_PIC();
  printf("PIC initialized\n");

  // Set up all the IDT gates
  set_idt_gates();
  printf("IDT gates setup\n");

  // Load the IDT
  idt_flush((u32)&idtr);

  return 0; // Success
}
