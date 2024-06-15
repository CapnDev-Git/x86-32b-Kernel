#include "gdt.h"

#include <k/types.h>
#include <stdio.h>
#include <string.h>

/**
 * \brief Load the Global Descriptor Table (GDT) into memory, ASM function in
 * gdt_flush.S
 * \param u32 The address of the GDT pointer structure (struct gdtr)
 */
extern void gdt_flush(u32 gdt_ptr);

/**
 * \brief Load the Task State Segment (TSS) into memory, ASM function in
 * gdt_flush.S
 */
extern void tss_flush(void);

/**
 * \brief Enable protected mode, ASM function in gdt_flush.S
 */
extern void enable_protected_mode(void);

/**
 * \brief Reload the segments, ASM function in gdt_flush.S
 */
extern void reload_segments(void);

#define NB_GDT_ENTRIES 6

#define SEGMENT_BASE_LOW(base) ((u32)(base)&0xFFFF)
#define SEGMENT_BASE_MIDDLE(base) (((u32)(base) >> 16) & 0xFF)
#define SEGMENT_BASE_HIGH(base) (((u32)(base) >> 24) & 0xFF)
#define SEGMENT_LIMIT(limit) ((u32)(limit)&0xFFFF)
#define SEGMENT_FLAGS(limit) (((u32)(limit) >> 16) & 0x0F)
#define SEGMENT_GRANULARITY(gran) ((u8)(gran)&0xF0)

#define SEGMENTS_BASE 0x00000000  // 0x00000000
#define SEGMENTS_LIMIT 0x000FFFFF // 1MB
#define SEGMENTS_FLAGS 0xCF       // Present, code/data, ring 0, read/write

#define CODE_SEGMENT_ACCESS 0x9A      // Present, code, ring 0, accessed
#define DATA_SEGMENT_ACCESS 0x92      // Present, data, ring 0, accessed
#define USER_CODE_SEGMENT_ACCESS 0xFA // Present, code, ring 3, accessed
#define USER_DATA_SEGMENT_ACCESS 0xF2 // Present, data, ring 3, accessed

#define TSS_ACCESS 0x89      // Present, code/data, ring 0, accessed
#define TSS_GRANULARITY 0x00 // 32-bit TSS
#define TSS_SS0 0x10         // Kernel data segment
#define TSS_ESP0 0x0         // Kernel stack pointer

/**
 * \brief Global Descriptor Table (GDT) entry
 * The GDT is a structure used by the CPU to store segment descriptors. The GDT
 * is loaded with the LGDT instruction and the CPU will automatically switch to
 * the GDT when the segment is loaded. The GDT is used to store information
 * about the segment such as the base address, limit, and access rights.
 * The GDT is used to store information about the segment such as the base
 * address, limit, and access rights.
 * \param limit The 16 bits of the limit
 * \param base_low The lower 16 bits of the base
 * \param base_middle The next 8 bits of the base
 * \param access Access flags, determine what ring this segment can be used in
 * \param flags Flags, determine the granularity and size of the segment
 * \param base_high The last 8 bits of the base
 */
struct gdt_entry {
  u16 limit;
  u16 base_low;
  u8 base_middle;
  u8 access;
  u8 flags;
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

/**
 * \brief Task State Segment (TSS) entry
 * The TSS is a structure used by the CPU to store information about a task. The
 * TSS is loaded with the LTR instruction and the CPU will automatically switch
 * to the TSS when the task is loaded. The TSS is used to store information
 * about the task such as the stack pointer, stack segment, and other registers.
 * The TSS is used to store information about the task such as the stack
 * pointer, stack segment, and other registers.
 *
 * \param prev_tss The previous TSS - if we used hardware task switching this
 * would form a linked list.
 * \param esp0 The stack pointer to load when we change to kernel mode.
 * \param ss0 The stack segment to load when we change to kernel mode.
 * \param esp1 Unused...
 * \param ss1
 * \param esp2
 * \param ss2
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
 * \param cs The value to load into CS when we change to kernel mode.
 * \param ss The value to load into SS when we change to kernel mode.
 * \param ds The value to load into DS when we change to kernel mode.
 * \param fs The value to load into FS when we change to kernel mode.
 * \param gs The value to load into GS when we change to kernel mode.
 * \param ldt Unused...
 * \param trap
 * \param iomap_base
 */
struct tss_entry {
  u32 prev_tss; // The previous TSS - if we used hardware task switching this
                // would form a linked list.
  u32 esp0;     // The stack pointer to load when we change to kernel mode.
  u32 ss0;      // The stack segment to load when we change to kernel mode.
  u32 esp1;     // Unused...
  u32 ss1;
  u32 esp2;
  u32 ss2;
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
  u32 es;  // The value to load into ES when we change to kernel mode.
  u32 cs;  // The value to load into CS when we change to kernel mode.
  u32 ss;  // The value to load into SS when we change to kernel mode.
  u32 ds;  // The value to load into DS when we change to kernel mode.
  u32 fs;  // The value to load into FS when we change to kernel mode.
  u32 gs;  // The value to load into GS when we change to kernel mode.
  u32 ldt; // Unused...
  u16 trap;
  u16 iomap_base;
} __attribute__((packed));

// TSS entry, GDT entries and GDTR
struct tss_entry tss_entry;
struct gdt_entry gdt_entries[NB_GDT_ENTRIES];
struct gdtr gdt_ptr;

/**
 * \brief Initialize a segment descriptor
 * \param entry The GDT entry to initialize
 * \param base The base address of the segment
 * \param limit The limit of the segment
 * \param flag The access flags for the segment
 */
static void set_gdt_gate(u32 num, u32 base, u32 limit, u8 access, u8 gran) {
  // Set the base addresses of the segment
  gdt_entries[num].base_low = SEGMENT_BASE_LOW(base);
  gdt_entries[num].base_middle = SEGMENT_BASE_MIDDLE(base);
  gdt_entries[num].base_high = SEGMENT_BASE_HIGH(base);

  // Set the limit of the segment (16 bits of the limit, 4 bits of flags)
  gdt_entries[num].limit = SEGMENT_LIMIT(limit);
  gdt_entries[num].flags = SEGMENT_FLAGS(limit);
  gdt_entries[num].flags |= SEGMENT_GRANULARITY(gran);

  // Set the access flags for the segment (ring level, present, type, etc.)
  gdt_entries[num].access = access;

  printf("GDT entry %d: base: 0x%08X, limit: 0x%08X, access: 0x%02X, flags: "
         "0x%02X\n",
         num, base, limit, access, gran);
}

/**
 * \brief Set the GDT gates
 */
static void set_gdt_gates(void) {
  set_gdt_gate(0, SEGMENTS_BASE, 0, 0, 0); // Null segment
  set_gdt_gate(1, SEGMENTS_BASE, SEGMENTS_LIMIT, CODE_SEGMENT_ACCESS,
               SEGMENTS_FLAGS); // Code segment
  set_gdt_gate(2, SEGMENTS_BASE, SEGMENTS_LIMIT, DATA_SEGMENT_ACCESS,
               SEGMENTS_FLAGS); // Data segment
  set_gdt_gate(3, SEGMENTS_BASE, SEGMENTS_LIMIT, USER_CODE_SEGMENT_ACCESS,
               SEGMENTS_FLAGS); // User mode code segment
  set_gdt_gate(4, SEGMENTS_BASE, SEGMENTS_LIMIT, USER_DATA_SEGMENT_ACCESS,
               SEGMENTS_FLAGS); // User mode data segment
}

/**
 * \brief Write the Task State Segment (TSS) entry to the GDT
 * \param num The number of the GDT entry to write the TSS to
 * \param ss0 The stack segment to load when we change to kernel mode
 * \param esp0 The stack pointer to load when we change to kernel mode
 */
static void write_tss(u32 num, u16 ss0, u32 esp0) {
  // Set the base and limit of the TSS to the address of the TSS entry
  u32 base = (u32)&tss_entry;
  u32 limit = base + sizeof(struct tss_entry);

  // Set the TSS entry in the GDT
  set_gdt_gate(num, base, limit, TSS_ACCESS, TSS_GRANULARITY);
  memset(&tss_entry, 0, sizeof(tss_entry)); // Clear the TSS entry

  // Set the stack segment and stack pointer for the TSS
  tss_entry.ss0 = ss0;
  tss_entry.esp0 = esp0;
  tss_entry.cs = 0x08 | 0x03;
  tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs =
      0x10 | 0x03;
}

/**
 * \brief Initialize the Global Descriptor Table (GDT)
 */
void init_gdt(void) {
  // Set the GDT pointer properties
  gdt_ptr.limit = (sizeof(struct gdt_entry) * NB_GDT_ENTRIES) - 1;
  gdt_ptr.base = (u32)&gdt_entries;

  // Set the GDT entries
  set_gdt_gates();

  // Write the TSS entry to the GDT
  write_tss(NB_GDT_ENTRIES - 1, TSS_SS0, TSS_ESP0);

  // Load the GDT with ASM in memory
  gdt_flush((u32)&gdt_ptr);
  printf("GDT loaded\n");

  // Load the TSS
  tss_flush();
  printf("TSS loaded\n");

  // Enable protected mode
  enable_protected_mode();
  printf("Protected mode enabled\n");
}
