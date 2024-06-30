#include <kstd.h>
#include <stddef.h>

/*
 * `syscallX_const` wrappers can be used when the syscall does *not* modify the
 * memory of the userland. This allows the compiler to optimize and reorder
 * memory accesses.
 *
 * In doubt use the `syscallX` wrappers that won't allow the compiler do do
 * assumption about the memory impact of a syscall. It is typically needed when
 * a buffer is passed to and modified by the kernel.
 */

static inline u32 syscall0(int syscall_nb) {
  u32 res;

  asm volatile("int $0x80" : "=a"(res) : "a"(syscall_nb));

  return res;
}

static inline u32 syscall1_const(int syscall_nb, u32 ebx) {
  u32 res;

  asm volatile("int $0x80" : "=a"(res) : "a"(syscall_nb), "b"(ebx));

  return res;
}

static inline u32 syscall1(int syscall_nb, u32 ebx) {
  u32 res;

  asm volatile("int $0x80" : "=a"(res) : "a"(syscall_nb), "b"(ebx) : "memory");

  return res;
}

static inline u32 syscall2_const(int syscall_nb, u32 ebx, u32 ecx) {
  u32 res;

  asm volatile("int $0x80" : "=a"(res) : "a"(syscall_nb), "b"(ebx), "c"(ecx));

  return res;
}

static inline u32 syscall2(int syscall_nb, u32 ebx, u32 ecx) {
  u32 res;

  asm volatile("int $0x80"
               : "=a"(res)
               : "a"(syscall_nb), "b"(ebx), "c"(ecx)
               : "memory");

  return res;
}

static inline u32 syscall3_const(int syscall_nb, u32 ebx, u32 ecx, u32 edx) {
  u32 res;

  asm volatile("int $0x80"
               : "=a"(res)
               : "a"(syscall_nb), "b"(ebx), "c"(ecx), "d"(edx));

  return res;
}

static inline u32 syscall3(int syscall_nb, u32 ebx, u32 ecx, u32 edx) {
  u32 res;

  asm volatile("int $0x80"
               : "=a"(res)
               : "a"(syscall_nb), "b"(ebx), "c"(ecx), "d"(edx)
               : "memory");

  return res;
}

int write(const void *s, size_t length) {
  return ((int)syscall2_const(SYSCALL_WRITE, (u32)s, length));
}

void *sbrk(ssize_t increment) {
  return ((void *)syscall1_const(SYSCALL_SBRK, increment));
}

int getkey(void) { return ((int)syscall0(SYSCALL_GETKEY)); }

unsigned long gettick(void) {
  return ((unsigned long)syscall0(SYSCALL_GETTICK));
}

int open(const char *pathname, int flags) {
  return ((int)syscall2_const(SYSCALL_OPEN, (u32)pathname, flags));
}

ssize_t read(int fd, void *buf, size_t count) {
  return ((ssize_t)syscall3(SYSCALL_READ, fd, (u32)buf, count));
}

off_t lseek(int filedes, off_t offset, int whence) {
  return ((off_t)syscall3_const(SYSCALL_SEEK, filedes, offset, whence));
}

int close(int fd) { return ((int)syscall1_const(SYSCALL_CLOSE, fd)); }

int playsound(struct melody *melody, int repeat) {
  return ((int)syscall2(SYSCALL_PLAYSOUND, (u32)melody, repeat));
}

int setvideo(int mode) { return ((int)syscall1_const(SYSCALL_SETVIDEO, mode)); }

void swap_frontbuffer(const void *buffer) {
  syscall1(SYSCALL_SWAP_FRONTBUFFER, (u32)buffer);
}

int getmouse(int *x, int *y, int *buttons) {
  return ((int)syscall3(SYSCALL_GETMOUSE, (u32)x, (u32)y, (u32)buttons));
}

void set_palette(unsigned int *new_palette, size_t size) {
  syscall2(SYSCALL_SETPALETTE, (u32)new_palette, size);
}
