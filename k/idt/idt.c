#include "idt.h"

#include "../io.h"
#include <k/types.h>
#include <stdio.h>
#include <string.h>

extern void isr0(void);

struct idt_gate {
  u16 base_low;
  u16 sel;
  u8 zero;
  u8 flags;
  u16 base_high;
} __attribute__((packed));

struct idtr {
  u16 limit;
  u32 base;
} __attribute__((packed));

extern void idt_flush(u32 idt_ptr);

#define NB_IDT_ENTRIES 256

struct idt_gate idt_entries[NB_IDT_ENTRIES];
struct idtr idtr;

static void set_idt_gate(u8 num, u32 base, u16 sel, u8 flags) {
  idt_entries[num].base_low = base & 0xFFFF;
  idt_entries[num].base_high = (base >> 16) & 0xFFFF;
  idt_entries[num].sel = sel;
  idt_entries[num].zero = 0;
  idt_entries[num].flags = flags | 0x60;
}

static void set_idt_gates(void) {
  set_idt_gate(0, (u32)isr0, 0x08, 0x8E);   // Divide by zero
  set_idt_gate(1, (u32)isr1, 0x08, 0x8E);   // Debug
  set_idt_gate(2, (u32)isr2, 0x08, 0x8E);   // NMI
  set_idt_gate(3, (u32)isr3, 0x08, 0x8E);   // Breakpoint
  set_idt_gate(4, (u32)isr4, 0x08, 0x8E);   // Overflow
  set_idt_gate(5, (u32)isr5, 0x08, 0x8E);   // BOUND Range Exceeded
  set_idt_gate(6, (u32)isr6, 0x08, 0x8E);   // Invalid Opcode
  set_idt_gate(7, (u32)isr7, 0x08, 0x8E);   // Device Not Available
  set_idt_gate(8, (u32)isr8, 0x08, 0x8E);   // Double Fault
  set_idt_gate(9, (u32)isr9, 0x08, 0x8E);   // Coprocessor Segment Overrun
  set_idt_gate(10, (u32)isr10, 0x08, 0x8E); // Invalid TSS
  set_idt_gate(11, (u32)isr11, 0x08, 0x8E); // Segment Not Present
  set_idt_gate(12, (u32)isr12, 0x08, 0x8E); // Stack-Segment Fault
  set_idt_gate(13, (u32)isr13, 0x08, 0x8E); // General Protection Fault
  set_idt_gate(14, (u32)isr14, 0x08, 0x8E); // Page Fault
  set_idt_gate(15, (u32)isr15, 0x08, 0x8E); // Unknown Interrupt
  set_idt_gate(16, (u32)isr16, 0x08, 0x8E); // Coprocessor Fault
  set_idt_gate(17, (u32)isr17, 0x08, 0x8E); // Alignment Check
  set_idt_gate(18, (u32)isr18, 0x08, 0x8E); // Machine Check
  set_idt_gate(19, (u32)isr19, 0x08, 0x8E); // SIMD Floating-Point Exception
  set_idt_gate(20, (u32)isr20, 0x08, 0x8E); // Virtualization Exception
  set_idt_gate(21, (u32)isr21, 0x08, 0x8E); // Control Protection Exception
  set_idt_gate(22, (u32)isr22, 0x08, 0x8E); // Reserved
  set_idt_gate(23, (u32)isr23, 0x08, 0x8E); // Reserved
  set_idt_gate(24, (u32)isr24, 0x08, 0x8E); // Reserved
  set_idt_gate(25, (u32)isr25, 0x08, 0x8E); // Reserved
  set_idt_gate(26, (u32)isr26, 0x08, 0x8E); // Reserved
  set_idt_gate(27, (u32)isr27, 0x08, 0x8E); // Reserved
  set_idt_gate(28, (u32)isr28, 0x08, 0x8E); // Reserved
  set_idt_gate(29, (u32)isr29, 0x08, 0x8E); // Reserved
  set_idt_gate(30, (u32)isr30, 0x08, 0x8E); // Reserved
  set_idt_gate(31, (u32)isr31, 0x08, 0x8E); // Reserved

  set_idt_gate(32, (u32)irq0, 0x08, 0x8E);  // Timer
  set_idt_gate(33, (u32)irq1, 0x08, 0x8E);  // Keyboard
  set_idt_gate(34, (u32)irq2, 0x08, 0x8E);  // Cascade
  set_idt_gate(35, (u32)irq3, 0x08, 0x8E);  // COM2
  set_idt_gate(36, (u32)irq4, 0x08, 0x8E);  // COM1
  set_idt_gate(37, (u32)irq5, 0x08, 0x8E);  // LPT2
  set_idt_gate(38, (u32)irq6, 0x08, 0x8E);  // Floppy
  set_idt_gate(39, (u32)irq7, 0x08, 0x8E);  // LPT1
  set_idt_gate(40, (u32)irq8, 0x08, 0x8E);  // CMOS
  set_idt_gate(41, (u32)irq9, 0x08, 0x8E);  // Free
  set_idt_gate(42, (u32)irq10, 0x08, 0x8E); // Free
  set_idt_gate(43, (u32)irq11, 0x08, 0x8E); // Free
  set_idt_gate(44, (u32)irq12, 0x08, 0x8E); // Mouse
  set_idt_gate(45, (u32)irq13, 0x08, 0x8E); // FPU
  set_idt_gate(46, (u32)irq14, 0x08, 0x8E); // Primary ATA
  set_idt_gate(47, (u32)irq15, 0x08, 0x8E); // Secondary ATA

  set_idt_gate(128, (u32)isr128, 0x08, 0x8E); // Syscall (0x80)
  set_idt_gate(177, (u32)isr177, 0x08, 0x8E); // Syscall (0xB1)
}

static void init_PIC(void) {
  // Start the PIC
  outb(0x20, 0x11);
  outb(0xA0, 0x11);

  // Set the PIC interrupt offsets
  outb(0x21, 0x20); // vector offset for master PIC
  outb(0xA1, 0x28); // vector offset for slave PIC

  // Set the PIC to 8086 mode
  outb(0x21, 0x04); // 0000 0100
  outb(0xA1, 0x02); // 0000 0010

  // Set the PIC to auto EOI mode
  outb(0x21, 0x01);
  outb(0xA1, 0x01);

  // Mask all interrupts
  outb(0x21, 0x0);
  outb(0xA1, 0x0);
}

static char *exception_messages[] = {"Division By Zero",
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
                                     "Reserved"};

struct iregs {
  u32 cr2;
  u32 ds;
  u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
  u32 int_no, err_code;
  u32 eip, cs, eflags, useresp, ss;
};

void isr_handler(struct iregs *regs) {
  printf("Received interrupt: %d\n", regs->int_no);

  if (regs->int_no < 32) {
    printf("-> Exception: %s\n", exception_messages[regs->int_no]);
    printf("-> Error code: %d\n", regs->err_code);
    printf("Processor halted!\n");
    for (;;)
      ;
  }
}

static void *irq_routines[16] = {0};
static void irq_install_handler(int irq, void (*handler)(struct iregs *r)) {
  irq_routines[irq] = handler;
}
static void irq_uninstall_handler(int irq) { irq_routines[irq] = 0; }

void irq_handler(struct iregs *regs) {
  // Get the handler from the IRQ routine
  void (*handler)(struct iregs * r);
  handler = irq_routines[regs->int_no - 32];

  // Call the handler if there is one associated with the IRQ
  if (handler)
    handler(regs);

  // Send an EOI (end of interrupt) signal to the PICs.
  if (regs->int_no >= 40)
    outb(0xA0, 0x20);

  // Send an EOI to the master PIC
  outb(0x20, 0x20);
}

void init_idt(void) {
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
}
