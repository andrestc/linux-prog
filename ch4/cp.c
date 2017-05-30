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
    int inputFd, outputFd, openFlags, keepHoles;
    mode_t filePerms;
    ssize_t numRead;
    char buf[BUF_SIZE], c;
 
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s [-k] file1 file2\n", argv[0]);

    while ((c = getopt(argc, argv, ":k")) != -1)
        switch(c)
        {
            case 'k':
                keepHoles = 1;
                break;
            default:
                errExit("invalid flag %c", c);
        }

    openFlags = O_RDONLY;
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH; /* rw-rw-rw- */

    inputFd = open(argv[optind], openFlags, filePerms);
    if (inputFd == -1)
        errExit("opening file %s %d", argv[optind], optind);
 
    optind++;
    openFlags = O_CREAT | O_WRONLY | O_TRUNC;

    outputFd = open(argv[optind], openFlags, filePerms);
    if (outputFd == -1)
        errExit("opening file %s %d", argv[optind], optind);
 
    int i, holes = 0;
    while ((numRead = read(inputFd, buf, BUF_SIZE)) > 0) 
    {
        if (keepHoles == 1) {
            for (i = 0; i < numRead; i++) {
                if (buf[i] == '\0') {
                    holes++;
                    continue;
                } else if (holes > 0) {
                    lseek(outputFd, holes, SEEK_CUR);
                    holes = 0;
                }
                if (write(outputFd, &buf[i], 1) != 1)
                    fatal("couldnt write char to file");
            }

        } else {
            if (write(outputFd, buf, numRead) != numRead)
                fatal("couldn't write whole buffer to file");
        }
    }
 
     if (close(outputFd) == -1)
        errExit("close output");

    if (close(inputFd) == -1)
        errExit("close input");

     exit(EXIT_SUCCESS);
}