.global gdt_flush
gdt_flush:
    mov 4(%esp), %eax       # Get the argument from the stack
    lgdt (%eax)             # Load the GDT descriptor
    call reload_segments    # Load the segment selectors
    ret                     # Return from the function

.global tss_flush
tss_flush:                 
    mov $0x28, %ax          # 0x2B is the offset in the GDT to our TSS descriptor
    ltr %ax                 # Load the TSS selector
    ret                     # Return from the function

.global enable_protected_mode
enable_protected_mode:
    mov %cr0, %eax        # Move the control register CR0 to EAX
    or $0x1, %eax         # Set the PE (Protection Enable) bit in CR0
    mov %eax, %cr0        # Move the modified value back to CR0
    ret                   # Return from the function

.global reload_segments
reload_segments:
    mov $0x10, %ax         # 0x10 is the offset in the GDT to our data segment
    mov %ax, %ds           # Load the data segment descriptor
    mov %ax, %es           # Load the data segment descriptor
    mov %ax, %fs           # Load the data segment descriptor
    mov %ax, %gs           # Load the data segment descriptor
    mov %ax, %ss           # Load the stack segment descriptor
    
    # Far jump to flush the prefetch queue and set the CS register
    ljmp $0x08, $.flush2     # 0x08 is the offset to our code segment
.flush2:
    ret                    # Return from the function



