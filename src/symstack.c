#include "symstack.h"

void symtable_stack_init(tSymTableStack *stack)
{
    stack->top = -1;
}

bool symtable_stack_is_empty(tSymTableStack *stack)
{
    return stack->top < 0;
}

void symtable_stack_push(tSymTableStack *stack, tSymTable *table)
{
    if (stack->top < MAX_SCOPE_DEPTH - 1)
    {
        stack->tables[++stack->top] = table;
        return;
    }
    {
        fprintf(stderr, "[INTERNAL] Fatal error: stack overflow\n");
        exit(INTERNAL_ERROR);
    }
}

void symtable_stack_pop(tSymTableStack *stack)
{
    if (symtable_stack_is_empty(stack))
        return;
    stack->top--;
}

tSymTable *symtable_stack_top(tSymTableStack *stack)
{
    if (symtable_stack_is_empty(stack))
    {
        fprintf(stderr,
                "[INTERNAL] Fatal error: attempt to access top of an empty symbol stack.\n");
        exit(INTERNAL_ERROR);
    }

    return stack->tables[stack->top];
}

void symtable_stack_free(tSymTableStack *stack)
{
    while (!symtable_stack_is_empty(stack))
        symtable_stack_pop(stack);
    stack->top = -1;
}
