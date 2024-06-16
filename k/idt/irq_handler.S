.macro IRQ irq_num irq_value    # Define a macro for an IRQ (2 arguments)
    .global irq\irq_num
    irq\irq_num:
        cli                    # Disable interrupts
        pushl $0               # Push a random value to align the stack
        pushl $\irq_value      # Push the IRQ number
        jmp irq_common_stub    # Jump to the common IRQ code
.endm

IRQ 0, 32                # System timer
IRQ 1, 33                # Keyboard
IRQ 2, 34                # Cascade for the second PIC
IRQ 3, 35                # COM2
IRQ 4, 36                # COM1
IRQ 5, 37                # LPT2
IRQ 6, 38                # Floppy disk
IRQ 7, 39                # LPT1
IRQ 8, 40                # CMOS real-time clock
IRQ 9, 41                # Free for peripherals / legacy SCSI / NIC
IRQ 10, 42               # Free for peripherals / SCSI / NIC
IRQ 11, 43               # Free for peripherals / SCSI / NIC
IRQ 12, 44               # PS/2 mouse
IRQ 13, 45               # FPU / coprocessor / inter-processor
IRQ 14, 46               # Primary ATA
IRQ 15, 47               # Secondary ATA

.extern irq_handler
irq_common_stub:
    pusha
    mov %ds, %eax
    push %eax
    mov %cr2, %eax
    push %eax

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp
    call irq_handler

    add $0x8, %esp
    pop %ebx
    mov %bx, %ds
    mov %bx, %es
    mov %bx, %fs
    mov %bx, %gs

    popa
    add $0x8, %esp
    sti
    iret
