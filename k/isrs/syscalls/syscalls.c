#include "syscalls.h"

#define NB_SYSCALLS 11

// TODO add getkey and gettick syscalls
#include "framebuffer.h" // setvideo, swap_frontbuffer
#include "isr.h"         // isr_install_handler
#include "serial.h"      // write
#include "syscalls_io.h" // open, read, seek, close
#include <k/types.h>     // u32
#include <stdio.h>       // printf

static u32 my_syscall(int ebx, int ecx, int edx) {
  (void)ebx;
  (void)ecx;
  (void)edx;
  printf("TEST SYSCALL REACHED!\n");
  return 42;
}

/**
 * \brief Syscall handler to write to the serial port
 * \param ebx The buffer to write to the serial port
 * \param ecx The number of bytes to write to the serial port
 * \param edx Unused
 * \return The number of bytes written to the serial port
 * \note See write() in serial.h
 */
static u32 write_handler(int ebx, int ecx, int edx) {
  (void)edx;
  return write((const char *)ebx, ecx);
}

// static u32 getkey_handler(int ebx, int ecx, int edx) {
//   (void)ebx;
//   (void)ecx;
//   (void)edx;
//   return getkey();
// }

// static u32 gettick_handler(int ebx, int ecx, int edx) {
//   (void)ebx;
//   (void)ecx;
//   (void)edx;
//   return gettick();
// }

/**
 * \brief Syscall handler to open a file
 * \param ebx The path to the file to open
 * \param ecx The flags to use when opening the file
 * \param edx Unused
 * \return The file descriptor of the opened file
 * \note See open() in syscalls_io.c
 */
static u32 open_handler(int ebx, int ecx, int edx) {
  (void)edx;
  return open((const char *)ebx, ecx);
}

/**
 * \brief Syscall handler to read from a file
 * \param ebx The file descriptor of the file to read from
 * \param ecx The buffer to read the file contents into
 * \param edx The number of bytes to read from the file
 * \return The number of bytes read from the file
 * \note See read() in syscalls_io.c
 */
static u32 read_handler(int ebx, int ecx, int edx) {
  return read(ebx, (void *)ecx, edx);
}

/**
 * \brief Syscall handler to seek in a file
 * \param ebx The file descriptor of the file to seek in
 * \param ecx The offset to seek to in the file
 * \param edx The reference point to seek from
 * \return The new file offset
 * \note See seek() in syscalls_io.c
 */
static u32 seek_handler(int ebx, int ecx, int edx) {
  return seek(ebx, ecx, edx);
}

/**
 * \brief Syscall handler to close a file
 * \param ebx The file descriptor of the file to close
 * \param ecx Unused
 * \param edx Unused
 * \return 0 on success, -1 on failure
 * \note See close() in syscalls_io.c
 */
static u32 close_handler(int ebx, int ecx, int edx) {
  (void)ecx;
  (void)edx;
  return close(ebx);
}

/**
 * \brief Syscall handler to set the video mode
 * \param ebx The video mode to set
 * \param ecx Unused
 * \param edx Unused
 * \return 0 on success, -1 on failure
 * \note See setvideo() in framebuffer.h
 */
static u32 setvideo_handler(int ebx, int ecx, int edx) {
  (void)ecx;
  (void)edx;
  printf("SET VIDEO SYSCALL REACHED!\n");
  return setvideo(ebx);
  return 0;
}

/**
 * \brief Syscall handler to swap the front buffer
 * \param ebx The buffer to swap to the front
 * \param ecx Unused
 * \param edx Unused
 * \return 0 on success, -1 on failure
 * \note See swap_frontbuffer() in framebuffer.h
 */
static u32 swap_frontbuffer_handler(int ebx, int ecx, int edx) {
  (void)ecx;
  (void)edx;
  printf("SWAP FRONTBUFFER SYSCALL REACHED!\n");
  swap_frontbuffer((const void *)ebx);
  return 0;
}

static u32 (*syscalls[])(int, int, int) = {
    my_syscall, // test syscall
    write_handler,
    NULL, // sbrk
    NULL, // getkey
    NULL, // gettick
    open_handler,  read_handler,     seek_handler,
    close_handler, setvideo_handler, swap_frontbuffer_handler};

/**
 * \brief ISR 128 - Syscall handler
 * \param iregs The interrupt registers with syscall number in eax, arguments in
 * ebx, ecx, edx
 * \note This function is called when a syscall interrupt is triggered. It
 * checks if the syscall number is valid and calls the corresponding syscall
 * function.
 * \note If the syscall number is invalid, the function returns -1.
 */
static void on_isr128(struct iregs *iregs) {
  printf("Received syscall: %d\n", iregs->eax);

  // Check if syscall number is valid
  if (iregs->eax < NB_SYSCALLS)
    // If syscall number is valid, call the corresponding syscall
    iregs->eax = syscalls[iregs->eax](iregs->ebx, iregs->ecx, iregs->edx);
  else
    // If syscall number is invalid, return -1
    iregs->eax = -1;
}

int init_syscalls(void) {
  // Install syscall handler
  if (isr_install_handler(128, &on_isr128) != 0) {
    printf("Failed to install syscall handler\n");
    return -1; // Failure
  }

  printf("Syscalls initialized\n");
  return 0; // Success
}
