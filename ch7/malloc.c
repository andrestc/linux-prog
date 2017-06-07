#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct block {
	size_t		size;
	struct block   *next;
	struct block   *prev;
}		block_t;

#ifndef ALLOC_UNIT
#define ALLOC_UNIT 3 * sysconf(_SC_PAGESIZE)
#endif

#ifndef MIN_DEALLOC
#define MIN_DEALLOC 1 * sysconf(_SC_PAGESIZE)
#endif

#define BLOCK_MEM(ptr) ((void *)((unsigned long)ptr + sizeof(block_t)))
#define BLOCK_HEADER(ptr) ((void *)((unsigned long)ptr - sizeof(block_t)))

static block_t *head = NULL;

void
fl_remove(block_t * b)
{
	if (!b->prev) {
		if (b->next) {
			head = b->next;
		} else {
			head = NULL;
		}
	} else {
		b->prev->next = b->next;
	}
	if (b->next) {
		b->next->prev = b->prev;
	}
}

void
fl_add(block_t * b)
{
	printf("adding %ld bytes block to free list\n", b->size);
	b->prev = NULL;
	b->next = NULL;
	if (!head || (unsigned long)head > (unsigned long)b) {
		if (head) {
			head->prev = b;
		}
		b->next = head;
		head = b;
	} else {
		block_t        *curr = head;
		while (curr->next && (unsigned long)curr->next < (unsigned long)b) {
			curr = curr->next;
		}
		b->next = curr->next;
		curr->next = b;
	}
}

//scans the list and merge continuous blocks
void
scan_merge()
{
	block_t        *curr = head;
	unsigned long	header_curr, header_next;
	unsigned long	program_break = (unsigned long)sbrk(0);
	if (program_break == 0) {
		printf("failed to retrieve program break\n");
		return;
	}
	while (curr->next) {
		header_curr = (unsigned long)curr;
		header_next = (unsigned long)curr->next;
		if (header_curr + curr->size + sizeof(block_t) == header_next) {
			printf("merging continuous blocks: %10p (size: %ld) <-> %10p (size: %ld) \n", curr, curr->size, curr->next, curr->next->size);
			curr->size += curr->next->size + sizeof(block_t);
			curr->next = curr->next->next;
			if (curr->next) {
				curr->next->prev = curr;
			} else {
				break;
			}
		}
		curr = curr->next;
	}
	stats("after merge");
	header_curr = (unsigned long)curr;
	printf("comparing last mem block %10p (size: %ld) to program break %10p\n", curr, curr->size, (void *)program_break);
	if (header_curr + curr->size + sizeof(block_t) == program_break && curr->size >= MIN_DEALLOC) {
		printf("releasing %ld bytes to OS\n", curr->size + sizeof(block_t));
		fl_remove(curr);
		if (brk(curr) != 0) {
			printf("error freeing memory\n");
		}
	}
}

//print debug stats
void
stats(char *prefix)
{
	printf("[%s] program break: %10p\n", prefix, sbrk(0));
	block_t        *ptr = head;
	printf("[%s] free list: \n", prefix);
	int		c = 0;
	while (ptr) {
		printf("(%d) <%10p> (size: %ld)\n", c, ptr, ptr->size);
		ptr = ptr->next;
		c++;
	}
}

//shrinks b to size and returns a new block with
// the rest of the size
block_t * split(block_t * b, size_t size)
{
	void           *mem_block = BLOCK_MEM(b);
	printf("splitting block at: %10p (size: %ld)\n", b, b->size);
	block_t        *newptr = (block_t *) ((unsigned long)mem_block + size);
	newptr->size = b->size - (size + sizeof(block_t));
	b->size = size;
	printf("created block at: %10p (size: %ld)\n", newptr, newptr->size);
	return newptr;
}

void           *
_malloc(size_t size)
{
	void           *block_mem;
	block_t        *ptr, *newptr;
	size_t		alloc_size = size >= ALLOC_UNIT ? size + sizeof(block_t) : ALLOC_UNIT;
	printf(">> _malloc: size %ld\n", size);
	ptr = head;
	while (ptr) {
		if (ptr->size >= size + sizeof(block_t)) {
			block_mem = BLOCK_MEM(ptr);
			fl_remove(ptr);
			if (ptr->size == size) {
				printf("reusing previously freed block: %10p\n", block_mem);
				return block_mem;
			}
			newptr = split(ptr, size);
			fl_add(newptr);
			return block_mem;
		} else {
			ptr = ptr->next;
		}
	}
	printf("unable to find available block. allocating: %ld bytes\n", alloc_size);
	ptr = sbrk(alloc_size);
	if (!ptr) {
		printf("failed to alloc %ld\n", alloc_size);
		return NULL;
	}
	ptr->next = NULL;
	ptr->prev = NULL;
	ptr->size = alloc_size - sizeof(block_t);
	if (alloc_size > size + sizeof(block_t)) {
		newptr = split(ptr, size);
		fl_add(newptr);
	}
	return BLOCK_MEM(ptr);
}

void
_free(void *ptr)
{
	fl_add(BLOCK_HEADER(ptr));
	stats("before scan");
	scan_merge();
}

void
_cleanup()
{
	printf("cleaning memory up\n");
	if (head) {
		if (brk(head) != 0) {
			printf("failed to cleanup memory");
		}
	}
	head = NULL;
	stats("_cleanup end");
}

int
main(int argc, char const *argv[])
{
	atexit(_cleanup);
	printf("mem page size: %ld bytes\n", sysconf(_SC_PAGESIZE));
	printf("bytes allocated per malloc: %ld\n", ALLOC_UNIT);
	stats("begin main");
	char           *str, *str2;
	str = (char *)_malloc(1);
	str2 = (char *)_malloc(1);
	_free(str);
	stats("1");
	str = (char *)_malloc(2);
	stats("2");
	_free(str2);
	_free(str);
	stats("end main");
	return (0);
}
