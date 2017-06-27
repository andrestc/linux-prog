/*
    Write a program that opens an existing file for writing with the O_APPEND flag, 
    and then seeks to the beginning of the file before writing some data. 
    Where does the data appear in the file? Why?

    => 
    If the O_APPEND file status flag is set on the open file description,
    then a write(2) always moves the file offset to the end of the file,
    regardless of the use of lseek().
*/
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "tlpi_hdr.h"
#include "error_functions.c"

int main(int argc, char *argv[]) 
{
    int fd;
    off_t offset;

    if (argc != 3)
        usageErr("%s file text\n", argv[0]);

    fd = open(argv[1], O_APPEND | O_WRONLY, NULL);
    if (fd == -1)
        errExit("opening file %s", argv[1]);

    offset = lseek(fd, 0, SEEK_SET);
    if ((int) offset == -1)
        errExit("lseek");

    if (write(fd, &argv[2][0], strlen(&argv[2][0])) != strlen(&argv[2][0]))
        errExit("write");

    exit(EXIT_SUCCESS);
}