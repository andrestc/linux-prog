#include <fcntl.h>
#include "tlpi_hdr.h"
#include "error_functions.c"
#include "get_num.c"

int main(int argc, char *argv[])
{
    int outputFd, openFlags;
    mode_t filePerms;
    off_t offset;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file <text> hole-length <text>\n", argv[0]);
    
    openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH; /* rw-rw-rw- */

    outputFd = open(argv[1], openFlags, filePerms);
    if (outputFd == -1)
        errExit("open");

    if (write(outputFd, &argv[2][0], strlen(&argv[2][0])) != strlen(&argv[2][0]))
        errExit("failed to write whole buffer");

    offset = getLong(&argv[3][0], GN_ANY_BASE, &argv[3][0]);

    if (lseek(outputFd, offset, SEEK_CUR) == -1)
        errExit("lseek");

    if (write(outputFd, &argv[4][0], strlen(&argv[4][0])) != strlen(&argv[4][0]))
        errExit("failed to write whole buffer");

    if (close(outputFd) == -1)
        errExit("close output");

    exit(EXIT_SUCCESS);
}