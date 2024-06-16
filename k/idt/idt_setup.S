.global idt_flush
idt_flush:
    mov 4(%esp), %eax       # Get the argument from the stack
    lidt (%eax)             # Load the IDT descriptor
    sti                     # Enable interrupts
    ret                     # Return from the function
