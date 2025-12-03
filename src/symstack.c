/**
 * @file symstack.c
 *
 * IFJ25 project
 *
 * Stack for symbol tables, used for managing scopes.
 *
 * @author Lukáš Denkócy <xdenkol00>
 */

#include "symstack.h"

void symtable_stack_init(tSymTableStack *stack)
{
    stack->top = NULL;
}

bool symtable_stack_is_empty(tSymTableStack *stack)
{
    return stack->top == NULL;
}

void symtable_stack_push(tSymTableStack *stack, tSymTable *table)
{
    tSymTableStackNode *newNode = (tSymTableStackNode *)safeMalloc(sizeof(tSymTableStackNode));
    if (newNode == NULL)
    {
        fprintf(stderr, "[INTERNAL] FatalError: Stack memory allocation failed\n");
        exit(INTERNAL_ERROR);
    }

    newNode->table = table;
    newNode->next = stack->top;
    stack->top = newNode;
}

void symtable_stack_pop(tSymTableStack *stack)
{
    if (symtable_stack_is_empty(stack))
    {
        return;
    }

    tSymTableStackNode *nodeToPop = stack->top;
    stack->top = nodeToPop->next;
    free(nodeToPop);
}

tSymTable *symtable_stack_top(tSymTableStack *stack)
{
    if (symtable_stack_is_empty(stack))
    {
        fprintf(stderr, "[INTERNAL] FatalError: Attempt to access top of an empty symbol stack.\n");
        exit(INTERNAL_ERROR);
    }

    return stack->top->table;
}

void symtable_stack_free(tSymTableStack *stack)
{
    while (!symtable_stack_is_empty(stack))
    {
        symtable_stack_pop(stack);
    }
}

tSymbolData *symtable_stack_find(tSymTableStack *stack, const char *key)
{
    tSymTableStackNode *currentNode = stack->top;
    while (currentNode != NULL)
    {
        tSymbolData *data = symtable_find(currentNode->table, key);
        if (data != NULL)
        {
            return data;
        }
        currentNode = currentNode->next;
    }
    return NULL;
}
