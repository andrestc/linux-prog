#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct block {
    size_t size;
    struct block *next;
    struct block *prev;
} block_t;

#ifndef ALLOC_UNIT
#define ALLOC_UNIT 3 * sysconf(_SC_PAGESIZE)
#endif

#define BLOCK_MEM(ptr) ((void *)((unsigned long)ptr + sizeof(block_t)))
#define BLOCK_HEADER(ptr) ((void *)((unsigned long)ptr - sizeof(block_t)))

static block_t *head = NULL;

void fl_remove(block_t *b)
{
    if (!b->prev) {
        if(b->next) {
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

void fl_add(block_t *b) {
    b->prev = NULL; 
    b->next = NULL;
    if (head) {
        b->next = head;
        head->prev = b;
    }
    head = b;
}

// print debug stats
void stats(char *prefix)
{
    printf("[%s] program break: %10p\n", prefix, sbrk(0));
    block_t *ptr = head;
    printf("[%s] free list: \n", prefix);
    int c = 0;
    while (ptr) {
        printf("(%d) [%10p] -> [%10p] (size: %ld)\n", c, ptr, ptr + ptr->size, ptr->size);
        ptr = ptr->next;
        c++;
    }
}

// shrinks b to size and returns a new block with 
// the rest of the size
block_t *split(block_t *b, size_t size) {
    void *mem_block = BLOCK_MEM(b);
    printf("splitting block at: %10p\n", mem_block);
    block_t *newptr = (block_t *)((unsigned long)mem_block + size);
    newptr->size = b->size - size;
    b->size = size;
    return newptr;
}

void *_malloc(size_t size)
{
    void *block_mem;
    block_t *ptr, *newptr;
    size_t alloc_size = size >= ALLOC_UNIT ? size + sizeof(block_t) : ALLOC_UNIT;
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
        printf("failed to alloc %ldzn", alloc_size);
        return NULL;
    }
    ptr->next = NULL;
    ptr->prev = NULL;
    ptr->size = alloc_size;
    if (alloc_size > size + sizeof(block_t)) {
        newptr = split(ptr, size);
        fl_add(newptr);
    }
    return BLOCK_MEM(ptr);
}

void _free(void *ptr) 
{
    fl_add(BLOCK_HEADER(ptr));
}

int main(int argc, char const *argv[])
{
    printf("mem page size: %ld bytes\n", sysconf(_SC_PAGESIZE));
    printf("bytes allocated per malloc: %ld\n", ALLOC_UNIT);
    stats("begin");
    char *str, *str2;
    str = (char *) _malloc(15);
    str2 = (char *) _malloc(15);
    _free(str);
    stats("1");
    str = (char *) _malloc(10);
    stats("2");
    _free(str2);
    _free(str);
    stats("3");
    return(0);
}
