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

void *_malloc(size_t size)
{
    printf(">> _malloc: size %ld\n", size);
    block_t *ptr = head;
    while (ptr) {
        if (ptr->size >= size + sizeof(block_t)) {
            void *block_mem = BLOCK_MEM(ptr); 
            fl_remove(ptr);
            if (ptr->size == size) {
                printf("reusing previously freed block: %10p\n", block_mem);
                return block_mem;
            }
            printf("splitting block at: %10p\n", block_mem);
            block_t *newptr = ptr + size;
            newptr->size = ptr->size - size - sizeof(block_t);
            ptr->size = size;
            fl_add(newptr);
            return block_mem;
        }
        ptr = ptr->next;
    }
    printf("unable to find available block\n");
    ptr = sbrk(sizeof(block_t) + size);
    if (!ptr) {
        printf("failed to alloc %ldzn", sizeof(block_t)+size);
        return NULL;
    }
    ptr->size = size;
    ptr->next = NULL;
    ptr->prev = NULL;
    return BLOCK_MEM(ptr);
}

void _free(void *ptr) 
{
    fl_add(BLOCK_HEADER(ptr));
}

int main(int argc, char const *argv[])
{
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
