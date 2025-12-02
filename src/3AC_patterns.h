#ifndef AC_PATTERNS_H
#define AC_PATTERNS_H

#include "3AC.h"
#include "parser.h"

void generate_program_entrypoint(ThreeACList *list);

void generate_eof(ThreeACList *list);

void generate_while(FILE *file, tToken *currentToken, tSymTableStack *stack);

void generate_function_call(tSymTable *g_symtable, tSymTableStack *stack, tToken *func_token, FILE *file, tToken *currentToken);

void generate_return(FILE *file, tToken *currentToken, tSymTableStack *stack, bool isOneLine);

tDataType generate_ifj_write(tSymTableStack *stack);
tDataType generate_ifj_read_str(tSymTableStack *stack);
tDataType generate_ifj_str(tSymTableStack *stack);
tDataType generate_ifj_read_num(tSymTableStack *stack);
tDataType generate_ifj_floor(tSymTableStack *stack);
tDataType generate_ifj_length(tSymTableStack *stack);
tDataType generate_ifj_substring(tSymTableStack *stack);
tDataType generate_ifj_strcmp(tSymTableStack *stack);
tDataType generate_ifj_ord(tSymTableStack *stack);
tDataType generate_ifj_chr(tSymTableStack *stack);



void generate_truthiness_check(ThreeACList *list, Operand *condition_result);

void generate_numeric_op(ThreeACList *list, char* op);
void generate_mult_op(ThreeACList *list);
void generate_add_op(ThreeACList *list);

void generate_relational_op(ThreeACList *list, char* op);

void generate_string_mult(ThreeACList *list);

#endif
