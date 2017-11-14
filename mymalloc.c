#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include "mymalloc.h"

static struct Block *head = NULL;	// head of list
static struct Block *tail = NULL;	// tail of list

void *my_malloc(int size)
{
    if(head == NULL)
    {
        head = sbrk(sizeof(struct Block));
        void *ptr = sbrk(size);
        head->occ = 1;
        head->size = size + sizeof(struct Block);
        head->prev = NULL;
        head->next = NULL;
        tail = head;
        return ptr;
    }
    else
    {
        struct Block *bestfit = head;
        int lowest = -1;
        struct Block *currentLowest;

        while(bestfit->next != NULL)
        {
            if(bestfit->occ == 0 && bestfit->size == (size + sizeof(struct Block)))
                return bestfit;

            if(bestfit->occ == 0 && (bestfit->size >= (size + sizeof(struct Block))) && (lowest > bestfit->size || lowest == -1))
            {
                lowest = bestfit->size;
                currentLowest = bestfit;
            }

            bestfit = bestfit->next;
        }

        if(lowest == -1)
        {
            struct Block *node = sbrk(sizeof(struct Block));
            void *ptr = sbrk(size);
            node->occ = 1;
            node->size = size + sizeof(struct Block);
            node->prev = bestfit;
            node->prev->next = node;
            node->next = NULL;
            tail = head;
            return ptr;
        }

        if(lowest > sizeof(struct Block) * 2 + size)
        {
            //split
        }

        else
            return currentLowest;

    }

}

void my_free(void *data)
{
    //free shit here
}


void dump_heap()
{
    struct Block *cur;
    printf("brk: %p\n", sbrk(0));
    printf("head->");
    for(cur = head; cur != NULL; cur = cur->next) {
        printf("[%d:%d:%d]->", cur->occ, (char*)cur - (char*)head, cur->size);
        fflush(stdout);
        assert((char*)cur >= (char*)head && (char*)cur + cur->size <= (char*)sbrk(0)); // check that block is within bounds of the heap
        if(cur->next != NULL) {
            assert(cur->next->prev == cur); // if not last block, check that forward/backward links are consistent
            assert((char*)cur + cur->size == (char*)cur->next); // check that the block size is correctly set
        }
    }
    printf("NULL\n");
}