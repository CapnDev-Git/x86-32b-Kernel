include ../../config.mk

LIB	= libk.a
OBJS	= \
	  graphic.o \
	  malloc.o \
	  sound.o \
	  strdup.o \
	  syscalls.o \

DEPS = $(OBJS:.o=.d)

$(OBJS): CPPFLAGS += -MMD -Iinclude -I../../k/include/ -I../libc/include/
malloc.o: CFLAGS += -Wno-gnu-null-pointer-arithmetic -Wno-unused-but-set-variable
malloc.o: CPPFLAGS += -include malloc-k.h

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

clean:
	$(RM) $(OBJS) $(DEPS) $(LIB)

-include $(DEPS)
