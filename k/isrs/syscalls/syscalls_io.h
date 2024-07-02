#ifndef SYSCALLS_IO_H
#define SYSCALLS_IO_H

#include <k/kstd.h>
#include <k/types.h>

/**
 * \brief Syscall to open a file
 * \param pathname The path to the file to open
 * \param flags The flags to use when opening the file
 * \return The file descriptor of the opened file
 * \note This function is a syscall and is used to open a file in the filesystem
 * that the kernel is running on by parsing the path and mapping it to a file
 * in the ISO filesystem.
 */
int open(const char *pathname, int flags);

/**
 * \brief Syscall to read from a file
 * \param fd The file descriptor of the file to read from
 * \param buf The buffer to read the file contents into
 * \param count The number of bytes to read from the file
 * \return The number of bytes read from the file
 * \note This function is a syscall and is used to read from a file in the
 * filesystem that the kernel is running on by reading the file contents into
 * a buffer. The file descriptor is used to identify the file to read from.
 * The number of bytes to read is specified by the count parameter. The number
 * of bytes read from the file is returned by the function. If an error occurs,
 * -1 is returned.
 */
ssize_t read(int fd, void *buf, size_t count);

/**
 * \brief Syscall to seek in a file (change the file reading offset)
 * \param fd The file descriptor of the file to seek in
 * \param offset The offset to seek to in the file
 * \param whence The reference point to seek from
 * \return The new file offset
 * \note This function is a syscall and is used to seek in a file in the
 * filesystem that the kernel is running on by changing the file reading offset.
 * The file descriptor is used to identify the file to seek in. The offset
 * parameter specifies the new file reading offset. The whence parameter
 * specifies the reference point to seek from. The new file offset is returned
 * by the function. If an error occurs, -1 is returned.
 */
off_t seek(int fd, off_t offset, int whence);

/**
 * \brief Syscall to close a file
 * \param fd The file descriptor of the file to close
 * \return 0 on success, -1 on failure
 * \note This function is a syscall and is used to close a file in the
 * filesystem that the kernel is running on by releasing the file descriptor.
 * The file descriptor is used to identify the file to close. If the file is
 * successfully closed, 0 is returned. If an error occurs, -1 is returned.
 */
int close(int fd);

#endif /* ! SYSCALLS_IO_H */
