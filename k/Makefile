include ../config.mk

TARGET	= k
OBJS	= \
	  crt0.o \
	  k.o \
	  libvga.o \
	  list.o \
	  memory.o \
	  panic.o \
	  graphics.o \
	  serial/serial.o \
	  gdt/gdt.o \
	  gdt/gdt_setup.o \
	  idt/idt.o \
	  idt/idt_setup.o \
	  irqs/irq.o \
	  irqs/irq_handler.o \
	  irqs/timer/timer.o \
	  irqs/keyboard/keyboard.o \
	  isrs/isr.o \
	  isrs/isr_handler.o \

DEPS = $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): CPPFLAGS += -MMD -Iinclude  -I../libs/libc/include/ -Igdt -Iidt -Iserial -Iirqs -Iisrs
$(TARGET): CFLAGS += $(K_EXTRA_CFLAGS) -g
$(TARGET): LDFLAGS += -Wl,-Tk.lds
$(TARGET): LDLIBS = -L../libs/libc -lc
$(TARGET): $(OBJS)

install: $(TARGET)
	$(INSTALL) $(TARGET) $(INSTALL_ROOT)/$(TARGET)

clean:
	$(RM) $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)
