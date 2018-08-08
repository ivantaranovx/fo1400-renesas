
#include <sys/types.h>
#include <sys/stat.h>

int close(int fd)
{
    return 0;
}

int fstat(int fd, struct stat *statbuf)
{
    return 0;
}

off_t lseek(int fd, off_t offset, int whence)
{
    return 0;
}

ssize_t read(int fd, void *buf, size_t count)
{
    return 0;
}

ssize_t write(int fd, const void *buf, size_t count)
{
    return 0;
}

int isatty(int fd)
{
    return 0;
}
