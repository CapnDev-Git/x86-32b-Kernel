#include "gdt.h"

#include <stdio.h>
#include <string.h>

#include <k/types.h>

#define SEG_DESCTYPE(x)                                                        \
  ((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x) ((x) << 0x07) // Present
#define SEG_SAVL(x) ((x) << 0x0C) // Available for system use
#define SEG_LONG(x) ((x) << 0x0D) // Long mode
#define SEG_SIZE(x) ((x) << 0x0E) // Size (0 for 16-bit, 1 for 32)
#define SEG_GRAN(x)                                                            \
  ((x) << 0x0F) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define SEG_PRIV(x) (((x)&0x03) << 0x05) // Set privilege level (0 - 3)

#define SEG_DATA_RD 0x00        // Read-Only
#define SEG_DATA_RDA 0x01       // Read-Only, accessed
#define SEG_DATA_RDWR 0x02      // Read/Write
#define SEG_DATA_RDWRA 0x03     // Read/Write, accessed
#define SEG_DATA_RDEXPD 0x04    // Read-Only, expand-down
#define SEG_DATA_RDEXPDA 0x05   // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD 0x06  // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07 // Read/Write, expand-down, accessed
#define SEG_CODE_EX 0x08        // Execute-Only
#define SEG_CODE_EXA 0x09       // Execute-Only, accessed
#define SEG_CODE_EXRD 0x0A      // Execute/Read
#define SEG_CODE_EXRDA 0x0B     // Execute/Read, accessed
#define SEG_CODE_EXC 0x0C       // Execute-Only, conforming
#define SEG_CODE_EXCA 0x0D      // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC 0x0E     // Execute/Read, conforming
#define SEG_CODE_EXRDCA 0x0F    // Execute/Read, conforming, accessed

#define GDT_CODE_PL0                                                           \
  SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | SEG_LONG(0) | SEG_SIZE(1) |    \
      SEG_GRAN(1) | SEG_PRIV(0) | SEG_CODE_EXRD

#define GDT_DATA_PL0                                                           \
  SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | SEG_LONG(0) | SEG_SIZE(1) |    \
      SEG_GRAN(1) | SEG_PRIV(0) | SEG_DATA_RDWR

#define GDT_CODE_PL3                                                           \
  SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | SEG_LONG(0) | SEG_SIZE(1) |    \
      SEG_GRAN(1) | SEG_PRIV(3) | SEG_CODE_EXRD

#define GDT_DATA_PL3                                                           \
  SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | SEG_LONG(0) | SEG_SIZE(1) |    \
      SEG_GRAN(1) | SEG_PRIV(3) | SEG_DATA_RDWR

#define TSS_DESCRIPTOR_ACCESS 0x89
#define TSS_DESCRIPTOR_SELECTOR 5 // TSS descriptor index in GDT
#define GDT_ENTRIES 6

/**
 * \brief Task State Segment (TSS) entry
 * The TSS is a structure used by the CPU to store information about a task. It
 * is used by the CPU when switching between tasks. The TSS is loaded with the
 * LTR instruction and the CPU will automatically switch to the TSS when the
 * task is switched to. The TSS is used to store the stack pointer, stack
 * segment, and other information about the task.
 *
 * \param prev_tss The previous TSS - if we used hardware task switching this
 * would form a linked list.
 * \param reserved0 Reserved, must be zero
 * \param esp0 The stack pointer to load when we change to kernel mode.
 * \param ss0 The stack segment to load when we change to kernel mode.
 * \param reserved1 Reserved, must be zero
 * \param esp1 Unused...
 * \param ss1
 * \param reserved2
 * \param esp2
 * \param ss2
 * \param reserved3
 * \param cr3
 * \param eip
 * \param eflags
 * \param eax
 * \param ecx
 * \param edx
 * \param ebx
 * \param esp
 * \param ebp
 * \param esi
 * \param edi
 * \param es The value to load into ES when we change to kernel mode.
 * \param reserved4
 * \param cs The value to load into CS when we change to kernel mode.
 * \param reserved5
 * \param ss The value to load into SS when we change to kernel mode.
 * \param reserved6
 * \param ds The value to load into DS when we change to kernel mode.
 * \param reserved7
 * \param fs The value to load into FS when we change to kernel mode.
 * \param reserved8
 * \param gs The value to load into GS when we change to kernel mode.
 * \param reserved9
 * \param ldt Unused...
 * \param reserved10
 * \param trap
 * \param iomap_base
 */
struct tss_entry {
  u16 prev_tss;  // The previous TSS - if we used hardware task switching this
                 // would form a linked list.
  u16 reserved0; // Reserved, must be zero
  u32 esp0;      // The stack pointer to load when we change to kernel mode.
  u16 ss0;       // The stack segment to load when we change to kernel mode.
  u16 reserved1; // Reserved, must be zero
  u32 esp1;      // Unused...
  u16 ss1;
  u16 reserved2;
  u32 esp2;
  u16 ss2;
  u16 reserved3;
  u32 cr3;
  u32 eip;
  u32 eflags;
  u32 eax;
  u32 ecx;
  u32 edx;
  u32 ebx;
  u32 esp;
  u32 ebp;
  u32 esi;
  u32 edi;
  u16 es; // The value to load into ES when we change to kernel mode.
  u16 reserved4;
  u16 cs; // The value to load into CS when we change to kernel mode.
  u16 reserved5;
  u16 ss; // The value to load into SS when we change to kernel mode.
  u16 reserved6;
  u16 ds; // The value to load into DS when we change to kernel mode.
  u16 reserved7;
  u16 fs; // The value to load into FS when we change to kernel mode.
  u16 reserved8;
  u16 gs; // The value to load into GS when we change to kernel mode.
  u16 reserved9;
  u16 ldt; // Unused...
  u16 reserved10;
  u16 trap;
  u16 iomap_base;
} __attribute__((packed));

/**
 * \brief Global Descriptor Table (GDT) entry
 * The GDT is a structure used by the CPU to store segment descriptors. The GDT
 * is loaded with the LGDT instruction and the CPU will automatically switch to
 * the GDT when the segment is loaded. The GDT is used to store information
 * about the segment such as the base address, limit, and access rights.
 * The GDT is used to store information about the segment such as the base
 * address, limit, and access rights.
 * \param limit_low The lower 16 bits of the limit
 * \param base_low The lower 16 bits of the base
 * \param base_middle The next 8 bits of the base
 * \param access Access flags, determine what ring this segment can be used in
 * \param granularity Granularity flags, sets the unit size and limit scaling
 * \param base_high The last 8 bits of the base
 */
struct gdt_entry {
  u16 limit_low;
  u16 base_low;
  u8 base_middle;
  u8 access;
  u8 granularity;
  u8 base_high;
} __attribute__((packed));

/**
 * \brief Global Descriptor Table Register (GDTR)
 * The GDTR is a structure used by the CPU to store the location and size of the
 * GDT. The GDTR is loaded with the LGDT instruction and the CPU will
 * automatically switch to the GDT when the segment is loaded.
 * \param limit The size of the GDT
 * \param base The base address of the GDT
 */
struct gdtr {
  u16 limit;
  u32 base;
} __attribute__((packed));

// TSS entry, GDT entries and GDTR
struct tss_entry tss;
struct gdt_entry gdt[GDT_ENTRIES];
struct gdtr gdtr;

/**
 * \brief Initialize a segment descriptor
 * \param entry The GDT entry to initialize
 * \param base The base address of the segment
 * \param limit The limit of the segment
 * \param flag The access flags for the segment
 */
static void init_seg_desc(struct gdt_entry *entry, u32 base, u32 limit,
                          u16 flag) {
  u64 descriptor;

  // Create the high 32 bit segment
  descriptor = limit & 0x000F0000; // set limit bits 19:16
  descriptor |=
      (flag << 8) & 0x00F0FF00; // set type, p, dpl, s, g, d/b, l and avl fields
  descriptor |= (base >> 16) & 0x000000FF; // set base bits 23:16
  descriptor |= base & 0xFF000000;         // set base bits 31:24

  // Shift by 32 to allow for low part of segment
  descriptor <<= 32;

  // Create the low 32 bit segment
  descriptor |= base << 16;         // set base bits 15:0
  descriptor |= limit & 0x0000FFFF; // set limit bits 15:0

  *(u64 *)entry = descriptor;
}

/**
 * \brief Initialize the segment descriptors
 */
static void init_segment_descriptors(void) {
  init_seg_desc(&gdt[0], 0, 0, 0);                     // Null segment
  init_seg_desc(&gdt[1], 0, 0x000FFFFF, GDT_CODE_PL0); // Kernel code
  init_seg_desc(&gdt[2], 0, 0x000FFFFF, GDT_DATA_PL0); // Kernel data
  init_seg_desc(&gdt[3], 0, 0x000FFFFF, GDT_CODE_PL3); // User code
  init_seg_desc(&gdt[4], 0, 0x000FFFFF, GDT_DATA_PL3); // User data
}

/**
 * \brief Write the TSS to the GDT
 * \param idx The index of the TSS descriptor in the GDT
 * \param ss0 The kernel stack segment
 * \param esp0 The kernel stack pointer
 */
static void write_tss(int idx, u16 ss0, u32 esp0) {
  // Configure the TSS
  memset(&tss, 0, sizeof(tss));
  tss.ss0 = ss0;   // Set the kernel stack segment
  tss.esp0 = esp0; // Set the kernel stack pointer

  // Compute the base and limit of the TSS
  u32 base = (u32)(uptr)&tss;
  u32 limit = sizeof(struct tss_entry) - 1;

  // Add the TSS descriptor to the GDT
  gdt[idx].base_low = base & 0xFFFF;
  gdt[idx].base_middle = (base >> 16) & 0xFF;
  gdt[idx].base_high = (base >> 24) & 0xFF;
  gdt[idx].limit_low = limit & 0xFFFF;
  gdt[idx].granularity = (limit >> 16) & 0x0F;
  gdt[idx].access = TSS_DESCRIPTOR_ACCESS; // Set access bits
}

/**
 * \brief Load the GDT
 */
static void load_gdt(void) { asm volatile("lgdt (%0)" : : "r"(&gdtr)); }

/**
 * \brief Load the TSS selector
 * \param tss_selector The TSS selector
 */
static void load_tss_selector(u16 tss_selector) {
  asm volatile("mov %0, %%ax\n"
               "ltr %%ax\n" ::"m"(tss_selector));
}

/**
 * \brief Reload the segment registers
 */
static void reload_segment_registers(void) {
  write("Reloading segment registers\r\n", 29);
  asm volatile("mov $0x10, %%ax\n"
               "mov %%ax, %%ds\n"
               "mov %%ax, %%es\n"
               "mov %%ax, %%fs\n"
               "mov %%ax, %%gs\n"
               "mov %%ax, %%ss\n"
               "push $0x08\n" // CS selector
               "push $next\n" // CS offset
               "retf\n"
               "next:\n"
               :
               :
               : "memory");
}

/**
 * \brief Enable protected mode
 */
static void enable_protected_mode(void) {
  asm volatile("mov %%cr0, %%eax\n"
               "or $0x1, %%eax\n"
               "mov %%eax, %%cr0\n"
               :
               :
               : "%eax");
}

/**
 * \brief Initialize the Global Descriptor Table (GDT)
 */
void init_gdt(void) {
  // Initialize the segment descriptors
  init_segment_descriptors();

  // Load the GDT
  gdtr.limit = sizeof(struct gdt_entry) * GDT_ENTRIES - 1;
  gdtr.base = (u32)gdt;
  load_gdt();
  write("GDT loaded\r\n", 12);

  // Initialize TSS entry at index 5 (6th entry of GDT, 0x28 bytes in) & load
  write_tss(TSS_DESCRIPTOR_SELECTOR, 0x10, 0x0); // TSS in GDT
  load_tss_selector(TSS_DESCRIPTOR_SELECTOR << 3);
  write("TSS loaded\r\n", 13);

  // Enable protected mode
  enable_protected_mode();
  write("Protected mode enabled\r\n", 25);

  // Reload segment registers
  reload_segment_registers();
  write("Segment registers reloaded\r\n", 29);
}
