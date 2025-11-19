#ifndef IFJ_PARSER_H
#define IFJ_PARSER_H

#include "scanner.h"
#include "symstack.h"
#include "symtable.h"
#include "error.h"
#include "helper.h"
#include "expr_parser.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern tSymTable *global_symtable;

typedef struct {
    char *name;
    tDataType returnType;
    int paramCount;
    tDataType params[3];
} tBuiltinDef;

int parse_program(FILE *file);

void get_next_token(FILE *file, tToken *currentToken);
void parse_prolog(FILE *file, tToken *currentToken);
void parse_class_def(FILE *file, tToken *currentToken, tSymTableStack *stack);
void insert_builtin_functions();
void parse_func_list(FILE *file, tToken *currentToken, tSymTableStack *stack);
void parse_function_declaration(FILE *file, tToken *currentToken, tSymTableStack *stack);
void parse_getter(FILE *file, tToken *currentToken, tSymTableStack *stack, char *funcName);
void parse_setter(FILE *file, tToken *currentToken, tSymTableStack *stack, char *funcName);
void parse_statement(FILE *file, tToken *currentToken, tSymTableStack *stack);
void check_node_defined(tSymNode *node);
void check_undefined_functions();
int parse_parameter_list(FILE *file, tToken *currentToken, tSymTableStack *stack, char ***paramNames);
void parse_if_statement(FILE *file, tToken *currentToken, tSymTableStack *stack);
void parse_while_statement(FILE *file, tToken *currentToken, tSymTableStack *stack);
void parse_for_statement(FILE *file, tToken *currentToken, tSymTableStack *stack);
void parse_return_statement(FILE *file, tToken *currentToken, tSymTableStack *stack);
void parse_variable_declaration(FILE *file, tToken *currentToken, tSymTableStack *stack);
void parse_assignment_statement(FILE *file, tToken *currentToken, tSymTableStack *stack);
void parse_block(FILE *file, tToken *currentToken, tSymTableStack *stack, bool isFunctionBody);
void parse_function_call(FILE *file, tToken *currentToken, tSymTableStack *stack);
void parse_term(FILE *file, tToken *currentToken, tSymTableStack *stack);
void parse_ifj_call(FILE *file, tToken *currentToken, tSymTableStack *stack);

tSymbolData *find_data_in_stack(tSymTableStack *stack, const char *key);

#endif
