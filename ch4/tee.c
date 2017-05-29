#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "tlpi_hdr.h"
#include "error_functions.c"

#ifndef BUF_SIZE /* Allow "cc -D" to override definition */
#define BUF_SIZE 1024
#endif

int main(int argc, char *argv[])
{
    int inputFd, outputFd, openFlags, appendFlag;
    mode_t filePerms;
    ssize_t numRead;
    char buf[BUF_SIZE], c;
 
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s [-a] file\n", argv[0]);

    while ((c = getopt(argc, argv, ":a")) != -1)
        switch(c)
        {
            case 'a':
                appendFlag = 1;
                break;
            default:
                errExit("invalid flag %c", c);
        }
 
    openFlags = O_CREAT | O_WRONLY;
    if (appendFlag == 1)
        openFlags |= O_APPEND;
    else
        openFlags |= O_TRUNC;
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH; /* rw-rw-rw- */

    outputFd = open(argv[optind], openFlags, filePerms);
    if (outputFd == -1)
        errExit("opening file %s %d", argv[optind], optind);
 
    /* Transfer data until we encounter end of input or an error */
    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) 
    {
        if (write(outputFd, buf, numRead) != numRead)
            fatal("couldn't write whole buffer to file");
        
        if (write(STDOUT_FILENO, buf, numRead) != numRead)
            fatal("couldn't write whole buffer to stdout");
    }
 
     if (close(outputFd) == -1)
        errExit("close output");

     exit(EXIT_SUCCESS);
}