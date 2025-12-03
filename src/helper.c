/**
 * @file helper.c
 *
 * IFJ25 project
 *
 * File with helper function
 *
 * @author Jakub Králik <xkralij00>
 */

#include "error.h"
#include "helper.h"

void *safeMalloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL)
    {
        fprintf(stderr, "[INTERNAL] FatalError: Memory allocation failed\n");
        exit(INTERNAL_ERROR);
    }
    return ptr;
}

void *safeRealloc(void *block, size_t size)
{
    void *ptr = realloc(block, size);
    if (ptr == NULL)
    {
        fprintf(stderr, "[INTERNAL] FatalError: Memory reallocation failed\n");
        exit(INTERNAL_ERROR);
    }
    return ptr;
}
