/**
 * @file symtable.h
 *
 * IFJ25 project
 *
 * Implementation of a symbol table using an AVL tree.
 *
 * @author Lukáš Denkócy <xdenkol00>
 */

#ifndef IFJ_SYMTABLE_H
#define IFJ_SYMTABLE_H

#include "helper.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Enumeration of symbol types (variable, function, etc.).
 */
typedef enum
{
    SYM_VAR,
    SYM_FUNC,
    SYM_LABEL
} tSymbolType;

/**
 * Enumeration of data types in the language.
 */
typedef enum
{
    TYPE_NUM,
    TYPE_STRING,
    TYPE_NULL,
    TYPE_UNDEF
} tDataType;

/**
 * Structure holding all data associated with a symbol.
 */
typedef struct
{
    tSymbolType kind;
    tDataType dataType;
    char *unique_name;

    // Function-specific data
    bool defined;
    tDataType returnType;
    tDataType *paramTypes;
    char **paramNames;
    int paramCount;
} tSymbolData;

/**
 * A node in the AVL tree representing a symbol.
 */
typedef struct SymNode
{
    char *key;
    tSymbolData data;
    struct SymNode *left;
    struct SymNode *right;
    int height;
} tSymNode;

/**
 * The symbol table structure, represented by the root of an AVL tree.
 */
typedef struct
{
    tSymNode *root;
} tSymTable;

/**
 * Initializes an empty symbol table.
 *
 * @param t Pointer to the symbol table to initialize.
 */
void symtable_init(tSymTable *t);

/**
 * Frees all nodes and associated data in the symbol table.
 *
 * @param t Pointer to the symbol table to free.
 */
void symtable_free(tSymTable *t);

/**
 * Inserts a new symbol into the symbol table.
 *
 * @param t Pointer to the symbol table.
 * @param key The key (name) of the symbol.
 * @param data The data associated with the symbol.
 * @return True if insertion was successful, false if the key already exists.
 */
bool symtable_insert(tSymTable *t, char *key, tSymbolData data);

/**
 * Finds a symbol in the symbol table by its key.
 *
 * @param t Pointer to the symbol table.
 * @param key The key of the symbol to find.
 * @return A pointer to the symbol's data if found, otherwise NULL.
 */
tSymbolData *symtable_find(tSymTable *t, const char *key);

/**
 * Finds a function in the symbol table.
 *
 * @param sym Pointer to the symbol table.
 * @param key The mangled name of the function to find.
 * @return True if the function is found, false otherwise.
 */
bool symtable_find_function(tSymTable *sym, const char *key);

#endif // IFJ_SYMTABLE_H
