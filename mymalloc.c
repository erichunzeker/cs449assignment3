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
    del = data - sizeof(struct Block);

    if(del->prev != NULL)
        previousNode = del->prev;

    if(del->next != NULL)
        nextNode = del->next;

    if(previousNode == NULL && nextNode == NULL)
    {
        head = NULL;
        sbrk(-(del->size));
        return;
    }

    else if(nextNode == NULL)                                                                           //CASE: Freeing node at tail
    {
        int size = 0;

        if(previousNode->occ == 0)
        {
            if(previousNode->prev != root)                                                              //if there's a free spot behind it that isnt head
            {
                previousNode->prev->next = NULL;                                                        //set spot behind other free spot as the tail
                size = previousNode->size + del->size;                                                  //size to drop
            }

            else
            {
                sbrk(-(del->size + previousNode->size + root->size));                                   //else: head, middle, and freeing node are all dropped
                head = NULL;                                                                            //head is null
                return;
            }

        }
        else
        {
            previousNode->next = NULL;
            size = del->size;
        }
        sbrk(-size);                                                                                    //free what was dropped
        return;                                                                                         //return ------GOOD-------
    }

    else if(previousNode == NULL)                                                                       //CASE: Freeing node at head
    {
        int size = 0;

        if(nextNode->occ == 0)                                                                          //if next node is also free
        {
            if(nextNode->next != NULL)                                                                  //make sure its not the tail
            {
                size = nextNode->size + del->size;                                                      //size is both connecting nodes

                del->next = nextNode->next;                                                             //set nodes next to the next space
                del->next->prev = del;                                                                  //
                del->size = size;                                                                       //new size combined
                del->occ = 0;
            }
            else
            {
                sbrk(-(del->size + nextNode->size + root->size));                                       //head to tail is free - free free everything
                head = NULL;
                return;
            }

        }
        else                                                                                            //free node at head
            del->occ = 0;

        return;
    }

    else if(previousNode->occ == 0)                                                                     //CASE: Freeing node and coalescing with one behind it
    {

        int newSize = 0;

        if(nextNode->occ == 0)                                                                          //Node in front is empty too
        {
            newSize = previousNode->size + del->size + nextNode->size;                                  //size is all three combined

            if(nextNode->next != NULL && previousNode->prev != root)                                    //space on each side of cluster
            {
                nextNode = nextNode->next;
                previousNode = previousNode->prev;

                del->size = newSize;
                del->prev = previousNode;
                del->prev->next = del;
                del->next = nextNode;
                del->next->prev = del;
                del->occ = 0;
                return;
            }
            else if(nextNode->next == NULL && previousNode->prev == root)                               //three nodes
            {
                sbrk(-(newSize + root->size));                                                          //clear all
                head = NULL;
                return;
            }
            else if(nextNode->next == NULL)                                                             //two from tail
            {
                previousNode = previousNode->prev;
                sbrk(-(newSize));
                previousNode->next = NULL;
                return;
            }
            else if(previousNode->prev == root)                                                         //two from head
            {
                nextNode = nextNode->next;

                del->size = newSize;
                del->prev = root;
                del->prev->next = del;
                del->next = nextNode;
                del->occ = 0;
                return;
            }


        }
        else                                                                                            //only coalescing back node
        {
            newSize = previousNode->size + del->size;
            if(previousNode->prev != root)
            {
                previousNode = previousNode->prev;

                del->size = newSize;
                del->prev = previousNode;
                del->prev->next = del;
                del->occ = 0;
                return;
            }

            else
            {
                previousNode = root;

                del->size = newSize;
                del->prev = previousNode;
                del->next = nextNode;
                del->occ = 0;
                return;
            }
        }
        return;
    }

    else if(nextNode->occ == 0)                                                                         //CASE: freeing node and coalescing one in front
    {
        int newSize = nextNode->size + del->size;

        if(nextNode->next != NULL)                                                                      //not two from tail
        {
            nextNode = nextNode->next;

            del->size = newSize;
            del->next = nextNode;
            del->next->prev = del;
            del->occ = 0;
            return;
        }

        else
        {
            sbrk(-(newSize));                                                                           //two from tail, free whole cluster
            return;
        }
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