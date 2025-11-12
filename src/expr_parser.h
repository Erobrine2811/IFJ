#ifndef EXPR_PARSER_H
#define EXPR_PARSER_H

#include "scanner.h"
#include "symstack.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    E_MUL_DIV,
    E_PLUS_MINUS,
    E_REL,
    E_EQ_NEQ,
    E_IS,
    E_TYPE,
    E_LPAREN,
    E_RPAREN,
    E_ID,
    E_DOLLAR
} tSymbol;

typedef enum {
    PREC_LESS = '<',
    PREC_GREATER = '>',
    PREC_EQUAL = '=',
    PREC_ERROR = 'E'
} tPrec;

typedef struct ExprStackNode {
    tSymbol symbol;
    bool is_terminal;
    char *value;
    struct ExprStackNode *next;
} tExprStackNode;

typedef struct {
    tExprStackNode *top;
} tExprStack;

int parse_expression(FILE *file, tToken *currentToken, tSymTableStack *stack);

#endif
