/**
 * @file helper.c
 * 
 * IFJ25 project
 * 
 * File with helper function
 * 
 * @author Jakub Králik <xkralij00>
 */


 
#ifndef IFJ_HELPER_H
#define IFJ_HELPER_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define EOL '\n'

/**
 * Function to safely allocate memory. Works like malloc
 * but safely handles memory allocation errors with the exit code 99.
 * And prints an error message to stderr.
 * 
 * @param size Size of memory to allocate
 * @return Pointer to allocated memory
 */
void *safeMalloc(size_t size);



/**
 * Function to safely reallocate memory. Works like realloc
 * but safely handles memory reallocation errors with the exit code 99.
 * And prints an error message to stderr.
 * 
 * @param block Pointer to memory block to reallocate
 * @param size New size of memory block
 * @return Pointer to reallocated memory
 */
void *safeRealloc(void *block, size_t size);



#endif // IFJ_HELPER_H