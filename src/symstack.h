#ifndef SYMSTACK_H
#define SYMSTACK_H

#include "error.h"
#include "symtable.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SCOPE_DEPTH 100

typedef struct
{
    tSymTable *tables[MAX_SCOPE_DEPTH];
    int top;
} tSymTableStack;

void symtable_stack_init(tSymTableStack *stack);
void symtable_stack_push(tSymTableStack *stack, tSymTable *table);
void symtable_stack_pop(tSymTableStack *stack);
void symtable_stack_free(tSymTableStack *stack);
bool symtable_stack_is_empty(tSymTableStack *stack);
tSymTable *symtable_stack_top(tSymTableStack *stack);

#endif
