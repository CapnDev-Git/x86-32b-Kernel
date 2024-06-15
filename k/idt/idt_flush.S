.global idt_flush
idt_flush:
    mov 4(%esp), %eax       # Get the argument from the stack
    lidt (%eax)             # Load the IDT descriptor
    sti                     # Enable interrupts
    ret                     # Return from the function

.macro ISR_NOERRCODE num        # Define a macro for an ISR without an error code (1 argument)
    .global isr\num
    isr\num:
        cli                    # Disable interrupts
        pushl $0               # Push a random value to align the stack
        pushl $\num            # Push the ISR number
        jmp isr_common_stub    # Jump to the common ISR code
.endm

.macro ISR_ERRCODE num          # Define a macro for an ISR with an error code (1 argument)
    .global isr\num
    isr\num:
        cli                    # Disable interrupts
        pushl $\num            # Push the error code
        jmp isr_common_stub    # Jump to the common ISR code
.endm

.macro IRQ irq_num irq_value    # Define a macro for an IRQ (2 arguments)
    .global irq\irq_num
    irq\irq_num:
        cli                    # Disable interrupts
        pushl $0               # Push a random value to align the stack
        pushl $\irq_value      # Push the IRQ number
        jmp irq_common_stub    # Jump to the common IRQ code
.endm

# Define the ISRs
ISR_NOERRCODE 0         # Division by zero       
ISR_NOERRCODE 1         # Debug
ISR_NOERRCODE 2         # Non-maskable interrupt
ISR_NOERRCODE 3         # Breakpoint
ISR_NOERRCODE 4         # Overflow
ISR_NOERRCODE 5         # Bound range exceeded
ISR_NOERRCODE 6         # Invalid opcode
ISR_NOERRCODE 7         # Device not available
ISR_ERRCODE 8           # Double fault
ISR_NOERRCODE 9         # Coprocessor segment overrun
ISR_ERRCODE 10          # Invalid TSS 
ISR_ERRCODE 11          # Segment not present 
ISR_ERRCODE 12          # Stack-segment fault
ISR_ERRCODE 13          # General protection fault
ISR_ERRCODE 14          # Page fault
ISR_NOERRCODE 15        # Unknown interrupt
ISR_NOERRCODE 16        # Coprocessor fault
ISR_NOERRCODE 17        # Alignment check
ISR_NOERRCODE 18        # Machine check
ISR_NOERRCODE 19        # SIMD floating-point exception
ISR_NOERRCODE 20        # Virtualization exception 
ISR_NOERRCODE 21        # Control protection exception
ISR_NOERRCODE 22        # Reserved 
ISR_NOERRCODE 23        # Reserved 
ISR_NOERRCODE 24        # Reserved 
ISR_NOERRCODE 25        # Reserved 
ISR_NOERRCODE 26        # Reserved 
ISR_NOERRCODE 27        # Reserved 
ISR_NOERRCODE 28        # Reserved 
ISR_NOERRCODE 29        # Reserved 
ISR_NOERRCODE 30        # Reserved 
ISR_NOERRCODE 31        # Reserved 

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

ISR_NOERRCODE 128       # System call
ISR_NOERRCODE 177       # System call

.extern isr_handler
isr_common_stub:
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
    call isr_handler

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
