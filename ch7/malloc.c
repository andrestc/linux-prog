#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct block {
    size_t size;
    struct block *next;
    struct block *prev;
} block_t;

#define BLOCK_MEM(ptr) ((void *)((unsigned long)ptr + sizeof(block_t)))
#define BLOCK_HEADER(ptr) ((void *)((unsigned long)ptr - sizeof(block_t)))

static block_t *head = NULL;

// print debug stats
void stats(char *prefix)
{
    printf("[%s] program break: %10p\n", prefix, sbrk(0));
    block_t *ptr = head;
    printf("[%s] free list: \n", prefix);
    int c = 0;
    while (ptr != NULL) {
        printf("(%d) [%10p] size: %ld\n", c, ptr, ptr->size);
        ptr = ptr->next;
        c++;
    }
}

void *_malloc(size_t size)
{
    block_t *ptr = head;
    while (ptr) {
        if (ptr->size == size) {
            return BLOCK_MEM(ptr);
        }
        ptr = ptr->next;
    }
    printf("unable to find available block\n");
    ptr = sbrk(sizeof(block_t) + size);
    if (!ptr) {
        printf("failed to alloc %ld", sizeof(block_t)+size);
        exit(-1);
    }
    ptr->size = size;
    return BLOCK_MEM(ptr);
}

void _free(void *ptr) 
{
    stats("free - start");
    block_t *block = BLOCK_HEADER(ptr);
    block->prev = NULL;
    if(head) {
        block->next = head;
        head->prev = block;
        head = block;
    } else {
        block->next = NULL;
        head = block;
    }
    stats("free - end");
}

int main(int argc, char const *argv[])
{
    stats("start");
    char *str;
    stats("malloc - start");
    str = (char *) _malloc(15);
    stats("malloc - end");
    strcpy(str, "tutorialspoint");
    printf("String = %s,  Address = %u\n", str, *str);
    _free(str);
    return(0);
}
