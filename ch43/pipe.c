#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "tlpi_hdr.h"

void timespec_diff(struct timespec *start, struct timespec *stop,
                   struct timespec *result)
{
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }
}

int main(int argc, char *argv[])
{
	int pfd[2];
	int fd;
	int numBlocks, blockSize, numRead;
	char *buf = NULL;
	struct timespec begin, end, duration;
	if (argc != 3 || strcmp(argv[1], "--help") == 0)
		usageErr("%s numBlocks blockSize\n", argv[0]);

	numBlocks = getInt(argv[1], GN_GT_0, "numBlocks");
	blockSize = getInt(argv[2], GN_GT_0, "blockSize");

	if (!(buf = malloc(blockSize)))
		fatal("failed to alloc");

	fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
		errExit("open");

	if (read(fd, buf, blockSize) != blockSize)
		errExit("read");

	if(pipe(pfd) == -1)
		errExit("pipe");

	switch(fork()) {
	case -1:
		errExit("fork");
	case 0:
		if(close(pfd[1]) == -1)
			errExit("close - child");
		for (int i=0; i<numBlocks; i++) {
			numRead = read(pfd[0], buf, blockSize);
			if (numRead == -1)
				errExit("read");
		}
		if (close(pfd[0]) == -1)
			errExit("close");
		_exit(EXIT_SUCCESS);
	default:
		if (close(pfd[0]) == -1)
			errExit("close - parent");
		if (clock_gettime(CLOCK_MONOTONIC, &begin) == -1)
			errExit("clock_gettime");

		for (int i=0; i<numBlocks; i++) {
			if (write(pfd[1], buf, blockSize) != blockSize)
				fatal("parent - partial write");
		}
		if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
			errExit("clock_gettime");
		timespec_diff(&begin, &end, &duration);
		float secs = duration.tv_sec + duration.tv_nsec/1000000000.0;
		printf("took %9.6f secs\n", secs);
		if (close(pfd[1]) == -1)
			errExit("close");
		wait(NULL);
		exit(EXIT_SUCCESS);
	}
}
