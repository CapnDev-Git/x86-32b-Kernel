# EPITA Kernel Development Project 🖥️

## Introduction

Welcome to my repository for the [kernel course at EPITA](https://k.lse.epita.fr/). This project involves developing a kernel for an x86-32bit Intel processor.

## 🤝 Maintainer

- 🦈 Eliott Flechtner

## 🛠️ Project Overview

### 🧩 Dependencies

Here are the dependencies you'll need to build this project:

- gcc-multilib
- grub2
- libisoburn
- find

### 📂 Source Tree

Here's a quick look at the important files and directories:

- `Makefile` - Top-level Makefile
- `config.mk` - Build-system configuration
- `k/` - Kernel source folder
  - `elf.h` - ELF header
  - `crt0.S` - CRT0 for the kernel
  - `k.c` - Kernel entry point
  - `multiboot.h` - Multiboot Specification header
  - `k.lds` - LD script for the kernel binary
  - `memory.c` - Kernel memory allocator
  - `include/k/` - Kernel includes
    - `atapi.h` - ATAPI definitions
    - `kstd.h` - K standard definitions
    - `kfs.h` - KFS structures definitions
    - `types.h` - Kernel types definitions
- `roms/` - ROMs folder
  - `chichepong/` - Chichepong folder
  - `roms.lds` - LD script for ROM binaries
- `libs/` - SDK folder
  - `libc/` - Basic libc available everywhere
  - `libk/` - Userland functions
- `tools/` - Tools folder
  - `mkksf` - Generate your own sounds
  - `mkkfs` - Create KFS ROMs
  - `create-iso.sh` - Generate the ISO image

### 📚 Intel Manuals

For detailed information on the processor and system programming, check out the Intel Manuals on the [Intel website](http://www.intel.com/products/processor/manuals/). The "Volume 3A: System Programming Guide" is especially useful for OS development.

## 🚀 Build System

### 🔧 Build Commands

Here's how to build and run the project:

- `make` | `make k.iso` - Create an ISO with all the ROMs
- `make k` - Compile the kernel
- `make rom/GAME` - Compile the ROM in the folder `rom/$(GAME)`
- `make clean` - Clean the tree

## 💻 Booting the Kernel

To boot the kernel in QEMU, use:

```bash
qemu-system-x86_64 -cdrom k.iso [ -enable-kvm ]
```


## 🐛 Debugging the Kernel

### 📊 Debugging with GDB

To debug the kernel with GDB, use:

1. Run QEMU with a GDB server and stop the CPU at the first instruction:

```bash
qemu-system-x86_64 -cdrom k.iso -s -S
```

2. Run GDB with the kernel binary:

```bash
gdb k/k
```

3. Connect GDB to the QEMU GDB server:

```gdb
target remote localhost:1234
```

4. Set a breakpoint and continue execution:

```gdb
b my_symbol
```

5. Run the simulation in GDB:

```gdb
continue
```
