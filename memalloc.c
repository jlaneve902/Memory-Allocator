#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

/**
 * Union used to track size and whether or not block of memory is free to overwrite with new memory.
 */
typedef union header{
    struct {
        size_t size;
        unsigned is_free;
        union header *next;
    }s;
    _Alignas(16) char stub[16]; //Guaranteed to align headers on mutlitples of 16.
}header_t;

//Pointers set to look at top and bottom of contigous chunk of memory.
header_t* head = NULL, *tail = NULL;

//Prevents concurrent access of memory, when accessing only one thread can have lock.
pthread_mutex_t global_malloc_lock;

/**
 * Before allocating new memory, check to see if a chunk of memory already exists that can fit desired size
 * of new request.
 */
header_t* get_free_block(size_t size){
    header_t *curr = head;
    while(curr){
        if(curr->s.is_free && curr->s.size >= size){
            return curr;
        }
        curr = curr->s.next;
    }
    return NULL;
}

/**
 * Allocates new memory on the heap. Returns pointer to block of memory.
 */
void* malloc(size_t size){
    size_t total_size;
    void* block;
    header_t* header;
    if(!size){ //Size is 0 check.
        return NULL;
    }

    /**Current thread takes key so no others can view free blocks concurrently**/
    pthread_mutex_lock(&global_malloc_lock);
    header = get_free_block(size);

    if(header){
        header->s.is_free = 0;

        /**Gives key back for other threads to use.**/
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header + 1);
    }

    total_size = sizeof(header_t) + size;
    block = sbrk(total_size);

    /**Check if valid memory address was returned.**/
    if(block == (void*)-1){

        /**Gives key back for other threads to use**/
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }

    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;

    if(!head){
        head = header;
    }
    if(tail){
        tail->s.next = header;
    }
    tail = header;

    /**Gives key back for other threads to use**/
    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header+1);
}