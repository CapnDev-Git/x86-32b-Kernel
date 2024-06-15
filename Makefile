include config.mk

ROMS	= \
	  roms/chichehunter \
	  roms/chichepong \
	  roms/chichevaders \
	  roms/perrodlauncher \
	  roms/skate \
	  roms/yakanoid \


SUBDIRS	= \
	  $(ROMS) \
	  k \
	  libs/libc \
	  libs/libk \
	  tools/mkkfs \

ABS_INSTALL = $(abspath $(INSTALL_ROOT))

all: k.iso

k: libs/libc

$(ROMS): tools/mkkfs libs/libc libs/libk

k.iso: install
	./tools/create-iso.sh $@ $(INSTALL_ROOT) $(ROMS)

$(SUBDIRS):
	$(MAKE) -C $@

install: libs/libc libs/libk k
	mkdir -p $(ABS_INSTALL)
	for I in $(ROMS);			\
	do					\
		$(MAKE) INSTALL_ROOT=$(ABS_INSTALL) -C $$I $@ || exit 1;	\
	done
	$(MAKE) INSTALL_ROOT=$(ABS_INSTALL) -C k $@

clean:
	for I in $(SUBDIRS);			\
	do					\
		$(MAKE) -C $$I $@ || exit 1;	\
	done
	$(RM) k.iso
	$(RM) -r root
	$(RM) -r iso

.PHONY: $(SUBDIRS) $(INSTALL_ROOT) install
