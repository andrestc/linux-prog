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
    block_t *ptr = head;
    while (ptr) {
        if (ptr->size == size) {
            fl_remove(ptr);
            printf("reusing previously freed block: %10p\n", BLOCK_MEM(ptr));
            return BLOCK_MEM(ptr);
        }
        ptr = ptr->next;
    }
    printf("unable to find available block\n");
    ptr = sbrk(sizeof(block_t) + size);
    if (!ptr) {
        printf("failed to alloc %ld", sizeof(block_t)+size);
        return NULL;
    }
    ptr->size = size;
    ptr->next = NULL;
    ptr->prev = NULL;
    return BLOCK_MEM(ptr);
}

void _free(void *ptr) 
{
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
}

int main(int argc, char const *argv[])
{
    char *str, *str2;
    str = (char *) _malloc(15);
    str2 = (char *) _malloc(15);
    strcpy(str, "tutorialspoint");
    strcpy(str2, "tutorial");
    printf("String = %s,  Address = %p\n", str, &str);
    printf("String = %s,  Address = %p\n", str2, &str2);
    _free(str);
    stats("1");
    str = (char *) _malloc(15);
    stats("2");
    _free(str2);
    _free(str);
    stats("3");
    return(0);
}
