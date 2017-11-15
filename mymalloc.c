#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include "mymalloc.h"

static struct Block *head = NULL;	// head of list
static struct Block *root;

void *my_malloc(int size)
{
    if(head == NULL)
    {
        head = sbrk(sizeof(struct Block));                                                  //expand heap by size of header
        root = head;
        void *ptr = sbrk(size);                                                             //make ptr start of malloc -> expand heap by size
        head->occ = 1;                                                                      //set header to occupied
        head->size = size + sizeof(struct Block);                                           //set header as size of whole block (data size + header)
        head->prev = NULL;                                                                  //single node - no prev
        head->next = NULL;
        return ptr;                                                                         //RETURN POINTER TO START OF MALLOC !!! NOT START OF HEADER !!!!!!!
    }
    else
    {
        struct Block *currentBlock = head;                                                  //set current block pointer to head
        int lowest = -1;
        struct Block *currentLowest;                                                        //keep track of free block with lowest wasted space

        while(currentBlock->next != NULL)                                                   //traverse linked list until end is reached
        {

            if(currentBlock->occ == 0 && currentBlock->size == (size + sizeof(struct Block)))
            {
                currentBlock->occ = 1;

                return ((char*)currentBlock + sizeof(struct Block));                        //current block is a perfect fit to malloc - return ptr to start of malloc
            }

            if(currentBlock->occ == 0 && (currentBlock->size >= (size + sizeof(struct Block))) && (lowest == -1 || currentBlock->size < lowest))
            {
                lowest = currentBlock->size;                                                //exact match wasnt found, but closest match for now is stored
                currentLowest = currentBlock;                                               //save current best fit
            }

            currentBlock = currentBlock->next;
        }

        if(lowest == -1)                                                                    //end of list is reached and no blocks were big enough
        {
            struct Block *node = sbrk(sizeof(struct Block));                                //allocate size of block to append to heap
            void *ptr = sbrk(size);                                                         //store pointer to start of data
            node->occ = 1;                                                                  //set occupied
            node->size = size + sizeof(struct Block);                                       //make size of node malloc plus original struct size
            node->prev = currentBlock;                                                      //make previous block the last of linked list
            node->prev->next = node;                                                        //set previous's next to itself
            node->next = NULL;
            return ptr;                                                                     //return pointer to start of malloc
        }

        if(lowest > sizeof(struct Block) * 2 + size)                                        //free spot was found, but bigger than needed and could store another set of data
        {
            struct Block *newFree = (char *)currentLowest + size + sizeof(struct Block);     //make a new node

            newFree->size = currentLowest->size - size - sizeof(struct Block);              //set size to what the current block won't use
            newFree->occ = 0;                                                               //set new node as free
            newFree->next = currentLowest->next;                                            //set new nodes next to large old blocks next
            newFree->next->prev = newFree;
            newFree->prev = currentLowest;                                                  //make current lowest the nodes prev

            currentLowest->size = size + sizeof(struct Block);                              //make size the malloc plus original struct size
            currentLowest->next = newFree;                                                  //reassure that the best fit's next node will be the new free node
            currentLowest->next->prev = currentLowest;
            currentLowest->occ = 1;
            return (currentLowest + sizeof(struct Block));
        }

        else
            return currentLowest;

    }

}

void my_free(void *data)
{

    struct Block *del, *previousNode = NULL, *nextNode = NULL;
    del = data - sizeof(struct Block);                                                      //del points to start of data

    if(del->prev != NULL)                                                                   //check to make sure it's not a head
        previousNode = del->prev;                                                           //set prev node

    if(del->next != NULL)                                                                   //check to make sure its not a tail
        nextNode = del->next;                                                               //set next node

    if(previousNode == NULL && nextNode == NULL)                                            //if del is the only node in the list
    {
        head = NULL;
        sbrk(-(del->size));                                                                 //size to decrement heap by is the size of data
        return;                                                                             //return because nothing left to do
    }

    else if(nextNode == NULL)                                                               //if user input is a tail
    {
        int size = 0;                                                                       //amount to decrease heap by

        if(previousNode->occ == 0)                                                          //previous space is also free, coalesce (cant be only node since i already checked that)
        {
            if(previousNode->prev != root)
            {
                previousNode->prev->next = NULL;
                size = previousNode->size + del->size;
            }
                                                                                        //set last occupied block as tail, deallocate free space
            else
            {
                sbrk(-(del->size + previousNode->size + root->size));
                head = NULL;                                                               //size to decrement heap by is the size of data
                return;
            }

        }
        else                                                                                //has to be previousNode->occ == 1 bc i checked to make sure it wasnt null
        {
            previousNode->next = NULL;                                                      //previous space becomes new tail
            size = del->size;                                                          //set size to deallocate heap by
        }
        sbrk(-size);
        return;
    }

    else if(previousNode == NULL)                                                           //if user input is a head
    {
        int size = 0;                                                                       //new size of free block

        if(nextNode->occ == 0)                                                              //next space is also free, coalesce (cant be only node since i already checked that)
        {

            size = nextNode->size + del->size;                                              //size of first and second block
            del->size = size;                                                               //set size
            del->next = nextNode->next;                                                     //ditch old free block since it joined with current
            del->prev = NULL;
            del->occ = 0;                                                                   //its free
            nextNode->next->prev = del;                                                     //connect next block to new big free block
        }
        else                                                                                //has to be nextNode->occ == 1 bc i checked to make sure it wasnt null
        {
            size = del->size;                                                               //size of everything
            nextNode->size = size;                                                          //new size set
            del->next = nextNode;
            del->prev = NULL;
            del->occ = 0;

        }
        return;
    }

    else if(previousNode->occ == 0)                                                         //block behind future free block is also free, coalesce
    {

        int newSize = 0;

        if(nextNode->occ == 0)                                                              //surrounding blocks are free - coalesce (previous and next)
        {
            newSize = previousNode->size + del->size + nextNode->size;
            nextNode = nextNode->next;                                                      //next node should be the occ in front of next free block
            previousNode = previousNode->prev;                                              //previous node should be the occ behind the previous free block

            del->size = newSize;                                                            //set new size to sum of previous, current, and next sizes
            del->prev = previousNode;                                                       //set new prev
            del->next = nextNode;                                                           //set new next
            del->occ = 0;                                                                   //set as free

        }
        else                                                                                //only previous block is free
        {
            newSize = previousNode->size + del->size;
            previousNode = previousNode->prev;              //previous node should now be behind the free previous block

            del->size = newSize;                            //set size as sum of freed block and previous free block
            del->prev = previousNode;                       //set new prev
            del->next = nextNode;                           //set new next
            del->occ = 0;                                   //set as free
        }
        return;
    }

    else if(nextNode->occ == 0)                             //only next block is free, coalesce
    {

        int newSize = nextNode->size + del->size;
        nextNode = nextNode->next;                          //previous node should now be behind the free previous block

        del->size = newSize;                                //set size as sum of freed block and previous free block
        del->prev = previousNode;                           //set new prev
        del->next = nextNode;                               //set new next
        del->occ = 0;                                       //set as free
        return;
    }

    else if(previousNode->occ == 1 && nextNode->occ == 1)
    {
        del->occ = 0;
        return;
    }

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