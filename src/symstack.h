/**
 * @file symstack.h
 *
 * IFJ25 project
 *
 * Stack for symbol tables, used for managing scopes.
 *
 * @author Lukáš Denkócy <xdenkol00>
 */

#ifndef IFJ_SYMSTACK_H
#define IFJ_SYMSTACK_H

#include "error.h"
#include "symtable.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * A node in the symbol table stack, representing one scope.
 */
typedef struct SymTableStackNode
{
    tSymTable *table;
    struct SymTableStackNode *next;
} tSymTableStackNode;

/**
 * The symbol table stack structure.
 */
typedef struct
{
    tSymTableStackNode *top;
} tSymTableStack;

/**
 * Initializes an empty symbol table stack.
 *
 * @param stack Pointer to the stack to be initialized.
 */
void symtable_stack_init(tSymTableStack *stack);

/**
 * Pushes a new symbol table onto the stack.
 *
 * @param stack Pointer to the stack.
 * @param table Pointer to the symbol table to push.
 */
void symtable_stack_push(tSymTableStack *stack, tSymTable *table);

/**
 * Pops the top symbol table from the stack and frees the stack node.
 * Note: This does not free the tSymTable pointer itself.
 *
 * @param stack Pointer to the stack.
 */
void symtable_stack_pop(tSymTableStack *stack);

/**
 * Frees all nodes in the stack.
 * Note: This does not free the tSymTable pointers held by the nodes.
 *
 * @param stack Pointer to the stack to be freed.
 */
void symtable_stack_free(tSymTableStack *stack);

/**
 * Checks if the symbol table stack is empty.
 *
 * @param stack Pointer to the stack.
 * @return True if the stack is empty, false otherwise.
 */
bool symtable_stack_is_empty(tSymTableStack *stack);

/**
 * Returns the symbol table at the top of the stack without popping it.
 *
 * @param stack Pointer to the stack.
 * @return Pointer to the top symbol table.
 */
tSymTable *symtable_stack_top(tSymTableStack *stack);

/**
 * Searches for a symbol through the entire stack, from top to bottom.
 *
 * @param stack Pointer to the stack to search in.
 * @param key The key of the symbol to find.
 * @return Pointer to the symbol's data if found, otherwise NULL.
 */
tSymbolData *symtable_stack_find(tSymTableStack *stack, const char *key);

#endif // IFJ_SYMSTACK_H
