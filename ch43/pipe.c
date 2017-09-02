#include <sys/wait.h>
#include "tlpi_hdr.h"

int main(int argc, char *argv[])
{
	int pfd[2];
	int numBlocks, blockSize, numRead;
	char *buf = NULL;
	if (argc != 3 || strcmp(argv[1], "--help") == 0)
		usageErr("%s numBlocks blockSize\n", argv[0]);

	if (!(buf = malloc(blockSize)))
		fatal("failed to alloc");

	if(pipe(pfd) == -1)
		errExit("pipe");

	switch(fork()) {
	case -1:
		errExit("fork");
	case 0:
		if(close(pfd[1]) == -1)
			errExit("close - child");
		for (;;) {
			numRead = read(pfd[0], buf, blockSize);
			if (numRead == -1)
				errExit("read");
			if (numRead == 0)
				break;
		}
		if (close(pfd[0]) == -1)
			errExit("close");
		_exit(EXIT_SUCCESS);
	default:
		if (close(pfd[0]) == -1)
			errExit("close - parent");
		for (int i=0; i<numBlocks; i++) {
			if (write(pfd[1], buf, blockSize) != blockSize)
				fatal("parent - partial write");
		}
		if (close(pfd[1]) == -1)
			errExit("close");
		wait(NULL);
		exit(EXIT_SUCCESS);
	}
}
