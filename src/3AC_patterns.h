#ifndef IFJ_3AC_PATTERNS_H
#define IFJ_3AC_PATTERNS_H

#include "3AC.h"
#include "parser.h"

void generate_program_entrypoint(tThreeACList *list);

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

void generate_truthiness_check(tThreeACList *list, tOperand *conditionResult);

void generate_numeric_op(tThreeACList *list, char *op);
void generate_mult_op(tThreeACList *list);
void generate_add_op(tThreeACList *list);

void generate_relational_op(tThreeACList *list, char *op);

void generate_string_mult(tThreeACList *list);

#endif // IFJ_3AC_PATTERNS_H
