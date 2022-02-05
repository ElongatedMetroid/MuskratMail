#include "common.h"

void *xcalloc(size_t multiplier, size_t size){
    void *result = calloc(multiplier, size);
    if(result == NULL)
        ERROR("error: unable to allocate memory!");
    return result;
}

void *xmalloc(size_t size){
    void *result = malloc(size);
    if(result == NULL)
        ERROR("error: unable to allocate memory!");
    return result;
}