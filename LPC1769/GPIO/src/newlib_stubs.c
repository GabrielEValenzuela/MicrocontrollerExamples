/**
 * \file newlib_stubs.c
 * @brief Newlib stubs for the project.
 * @details This file redefines common functions required by the newlib library to avoid linking errors
 *          in an embedded system environment without an operating system.
 *
 * @note Avoid modifying this file unless necessary. These implementations are minimal and may need
 *       to be expanded based on specific project requirements.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/unistd.h>

#include "LPC17xx.h"
#include "core_cm3.h"

#undef errno

extern int errno; //!< The `errno` variable is set by system calls and some library functions in the event of an error
                  //!< to indicate what went wrong. Each thread has its own error value, so `errno` is thread-local.

extern char _ebss; //!< This variable is defined by the linker script and marks the end of the BSS segment. It is used
                   //!< as the initial value of the heap in the `_sbrk` implementation.

static char* heap_end; //!< `heap_end` is a pointer to the current end of the heap, used by the `_sbrk` function to
                       //!< manage dynamic memory allocation.

char* __env[1] = {0}; //!< The `__env` array is a placeholder for environment variables. In this minimal implementation,
                      //!< it contains only a single `NULL` pointer, indicating that no environment variables are set.

char** environ = __env; //!< `environ` points to the `__env` array, providing access to the environment variables. In
                        //!< this implementation, it points to an empty environment list.

/**
 * @brief Writes data to a file descriptor.
 *
 * @param file File descriptor to write to.
 * @param ptr Pointer to the data buffer.
 * @param len Length of data to write.
 * @return Number of bytes written on success, -1 on error.
 */
int _write(int file, char* ptr, int len);

/**
 * @brief Exit the program.
 *
 * @param status Exit status code.
 * @note This function enters an infinite loop after attempting to write "exit" to STDOUT.
 */
void _exit(int status)
{
    _write(STDOUT_FILENO, "exit", 4);
    while (1)
    {
        // Infinite loop to prevent returning.
    }
}

/**
 * @brief Closes a file descriptor.
 *
 * @param file File descriptor to close.
 * @return -1 indicating failure as file descriptors are not used.
 */
int _close(int file)
{
    return -1;
}

/**
 * @brief Executes a program.
 *
 * @param name Name of the program to execute.
 * @param argv Argument vector.
 * @param env Environment variables.
 * @return -1 indicating failure as process control is not supported.
 */
int _execve(char* name, char** argv, char** env)
{
    errno = ENOMEM;
    return -1;
}

/**
 * @brief Creates a new process.
 *
 * @return -1 indicating failure as process creation is not supported.
 */
int _fork()
{
    errno = EAGAIN;
    return -1;
}

/**
 * @brief Gets file status.
 *
 * @param file File descriptor.
 * @param st Pointer to a stat structure to be filled.
 * @return 0 on success.
 */
int _fstat(int file, struct stat* st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

/**
 * @brief Gets the process ID.
 *
 * @return Process ID, which is always 1 in this implementation.
 */
int _getpid()
{
    return 1;
}

/**
 * @brief Checks if a file descriptor refers to a terminal.
 *
 * @param file File descriptor to check.
 * @return 1 if it is a terminal, 0 otherwise.
 */
int _isatty(int file)
{
    switch (file)
    {
        case STDOUT_FILENO:
        case STDERR_FILENO:
        case STDIN_FILENO: return 1;
        default: errno = EBADF; return 0;
    }
}

/**
 * @brief Sends a signal to a process.
 *
 * @param pid Process ID.
 * @param sig Signal number.
 * @return -1 indicating failure as signal handling is not supported.
 */
int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

/**
 * @brief Creates a new link (hard link) to an existing file.
 *
 * @param old Old path name.
 * @param new New path name.
 * @return -1 indicating failure as file system operations are not supported.
 */
int _link(char* old, char* new)
{
    errno = EMLINK;
    return -1;
}

/**
 * @brief Sets the position of the file offset.
 *
 * @param file File descriptor.
 * @param ptr Offset.
 * @param dir Direction (SEEK_SET, SEEK_CUR, SEEK_END).
 * @return 0 indicating success.
 */
int _lseek(int file, int ptr, int dir)
{
    return 0;
}

/**
 * @brief Increases program data space (heap).
 *
 * @param incr Number of bytes to increase heap by.
 * @return Pointer to the start of the new heap area on success, (caddr_t)-1 on error.
 */
caddr_t _sbrk(int incr)
{
    if (heap_end == 0)
    {
        heap_end = &_ebss;
    }

    char* prev_heap_end = heap_end;
    char* stack = (char*)__get_MSP();

    if (heap_end + incr > stack)
    {
        _write(STDERR_FILENO, "Heap and stack collision\n", 25);
        errno = ENOMEM;
        return (caddr_t)-1;
    }

    heap_end += incr;
    return (caddr_t)prev_heap_end;
}

/**
 * @brief Reads data from a file descriptor.
 *
 * @param file File descriptor to read from.
 * @param ptr Pointer to the buffer where data should be stored.
 * @param len Maximum number of bytes to read.
 * @return Number of bytes read on success, -1 on error.
 */
int _read(int file, char* ptr, int len)
{
    switch (file)
    {
        case STDIN_FILENO:
            // Implement input reading if necessary
            return 0;
        default: errno = EBADF; return -1;
    }
}

/**
 * @brief Gets file status by path.
 *
 * @param filepath Path to the file.
 * @param st Pointer to a stat structure to be filled.
 * @return 0 on success.
 */
int _stat(const char* filepath, struct stat* st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

/**
 * @brief Gets process times.
 *
 * @param buf Pointer to a tms structure to be filled.
 * @return (clock_t) -1 indicating failure as timing is not supported.
 */
clock_t _times(struct tms* buf)
{
    return (clock_t)-1;
}

/**
 * @brief Deletes a name from the file system.
 *
 * @param name Name of the file to delete.
 * @return -1 indicating failure as file system operations are not supported.
 */
int _unlink(char* name)
{
    errno = ENOENT;
    return -1;
}

/**
 * @brief Waits for a child process to change state.
 *
 * @param status Pointer to an integer to store the status information.
 * @return -1 indicating failure as process control is not supported.
 */
int _wait(int* status)
{
    errno = ECHILD;
    return -1;
}

/**
 * @brief Writes data to a file descriptor.
 *
 * @param file File descriptor to write to.
 * @param ptr Pointer to the data buffer.
 * @param len Length of data to write.
 * @return Number of bytes written on success, -1 on error.
 */
int _write(int file, char* ptr, int len)
{
    switch (file)
    {
        case STDOUT_FILENO:
        case STDERR_FILENO:
            // Implement output writing if necessary
            return len;
        default: errno = EBADF; return -1;
    }
}
