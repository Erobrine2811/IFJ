#ifndef IFJ_SYMTABLE_H
#define IFJ_SYMTABLE_H

#include "helper.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

typedef enum {
    SYM_VAR,
    SYM_FUNC,
    SYM_LABEL
} tSymbolType;

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_NUM,
    TYPE_STRING,
    TYPE_NULL,
    TYPE_UNDEF
} tDataType;

typedef struct {
    tSymbolType kind;
    tDataType dataType;

    int index;

    bool defined;
    tDataType returnType;
    tDataType *paramTypes;
    char **paramNames;
    int paramCount;
} tSymbolData;

typedef struct SymNode {
    char *key;
    tSymbolData data;
    struct SymNode *left;
    struct SymNode *right;
    int height;
} tSymNode;

typedef struct {
    tSymNode *root;
} tSymTable;

void symtable_init(tSymTable *t);
void symtable_free(tSymTable *t);

bool symtable_insert(tSymTable *t, char *key, tSymbolData data);
tSymbolData *symtable_find(tSymTable *t, char *key);

#endif
