#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include "mymalloc.h"

static struct Block *head = NULL;	// head of list
static struct Blcok *tail = NULL;

void *my_malloc(int size)
{
    if(head == NULL)
    {
        head = sbrk(sizeof(struct Block));                                                  //expand heap by size of header
        void *ptr = sbrk(size);                                                             //make ptr start of malloc -> expand heap by size
        head->occ = 1;                                                                      //set header to occupied
        head->size = size + sizeof(struct Block);                                           //set header as size of whole block (data size + header)
        head->prev = NULL;                                                                  //single node - no prev
        head->next = NULL;                                                                  //single node - no next
        return ptr;                                                                         //RETURN POINTER TO START OF MALLOC !!! NOT START OF HEADER !!!!!!!
    }
    else
    {
        struct Block *bestfit = head;
        int lowest = -1;
        struct Block *currentLowest;

        while(bestfit->next != NULL)
        {
            bestfit = bestfit->next;

            if(bestfit->occ == 0 && bestfit->size == (size + sizeof(struct Block)))
                return bestfit + sizeof(struct Block);

            if(bestfit->occ == 0 && (bestfit->size >= (size + sizeof(struct Block))) && (lowest > bestfit->size || lowest == -1))
            {
                lowest = bestfit->size;
                currentLowest = bestfit;
            }
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
            struct Block *newFree;

            newFree->size = currentLowest->size - (size + sizeof(struct Block));
            newFree->occ = 0;
            newFree->next = currentLowest->next;
            newFree->prev = currentLowest;

            currentLowest->size = size + sizeof(struct Block);
            currentLowest->next = newFree;
        }

        else
            return currentLowest;

    }

}

void my_free(void *data) //SHOULD WORK CORRECTLY
{

    struct Block *del, *previousNode, *nextNode;

    del = data;


    if(del->prev == NULL && del->next == NULL)
    {
        int size = del->size + sizeof(struct Block);
        sbrk(-size);
    }

    if(del->prev != NULL)                                   //(Should be correct)
    {
        previousNode = del->prev;                           //set prev node
    }

    if(del->next != NULL)                                   //(Should be correct)
    {
        nextNode = del->next;                               //set next node
    }

    if(del->next == NULL )                                  //deallocate end of heap if user wants to free tail (should be correct)
    {
        int size = 0;

        if(previousNode->occ == 0)                          //previous space is also free, coalesce
        {
            previousNode->prev->next = NULL;                //set last occupied block as tail, deallocate free space
            size = previousNode->size + del->size + sizeof(struct Block);
        }
        else
        {
            previousNode->next = NULL;                      //previous space becomes new tail
            size = del->size + sizeof(struct Block);        //set size to deallocate heap by

        }
        sbrk(-size);

    }

    else if(previousNode->occ == 0)                         //block behind future free block is also free, coalesce
    {
        int newSize = 0;

        if(nextNode->occ == 0)                              //surrounding blocks are free - coalesce (previous and next)
        {
            newSize = previousNode->size + del->size + nextNode->size + sizeof(struct Block);
            nextNode = nextNode->next;                      //next node should be the occ in front of next free block
            previousNode = previousNode->prev;              //previous node should be the occ behind the previous free block

            del->size = newSize;                            //set new size to sum of previous, current, and next sizes
            del->prev = previousNode;                       //set new prev
            del->next = nextNode;                           //set new next
            del->occ = 0;                                   //set as free

        }
        else //only previous block is free
        {
            newSize = previousNode->size + del->size + sizeof(struct Block);
            previousNode = previousNode->prev;              //previous node should now be behind the free previous block

            del->size = newSize;                            //set size as sum of freed block and previous free block
            del->prev = previousNode;                       //set new prev
            del->next = nextNode;                           //set new next
            del->occ = 0;                                   //set as free
        }
    }

    else if(nextNode->occ == 0)                             //only next block is free, coalesce
    {
        int newSize = nextNode->size + del->size + sizeof(struct Block);
        nextNode = nextNode->next;                          //previous node should now be behind the free previous block

        del->size = newSize;                                //set size as sum of freed block and previous free block
        del->prev = previousNode;                           //set new prev
        del->next = nextNode;                               //set new next
        del->occ = 0;                                       //set as free
    }

    else                                                    //
    {
        del->occ = 0;
        int dealloc = del->size + sizeof(struct Block);
        sbrk(-dealloc);
    }


    return;

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