#include <stdio.h>
#include <unistd.h>

void* malloc(size_t size){
    void* chunk = sbrk(size);
    if(chunk == (void*) -1){
        return NULL;
    }
    return chunk;
}

struct header_t {
    size_t size;
    bool is_free;
};