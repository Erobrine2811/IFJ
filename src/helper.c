#include "helper.h"
#include "error.h"

void *safeMalloc(size_t size) 
{
    void *ptr = malloc(size);
    if (ptr == NULL)
    {
        fprintf(stderr, "[INTERNAL] Fatal error: memory allocation failed\n");
        exit(INTERNAL_ERROR);
    }
    return ptr;
}

void *safeRealloc(void *block, size_t size)
{
    void *ptr = realloc(block, size);
    if (ptr == NULL)
    {
        fprintf(stderr, "[INTERNAL] Fatal error: memory reallocation failed\n");
        exit(INTERNAL_ERROR);
    }
    return ptr;
}

