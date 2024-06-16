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
