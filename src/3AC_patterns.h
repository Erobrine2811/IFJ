#ifndef IFJ_3AC_PATTERNS_H
#define IFJ_3AC_PATTERNS_H

#include "3AC.h"
#include "parser.h"

void generate_program_entrypoint();

void generate_return(FILE *file, tToken *currentToken, tSymTableStack *stack, bool isOneLine);

tDataType generate_ifj_write();
tDataType generate_ifj_read_str();
tDataType generate_ifj_str();
tDataType generate_ifj_read_num();
tDataType generate_ifj_floor();
tDataType generate_ifj_length();
tDataType generate_ifj_substring();
tDataType generate_ifj_strcmp();
tDataType generate_ifj_ord();
tDataType generate_ifj_chr();

void generate_truthiness_check(tOperand *conditionResult);

void generate_numeric_op(char *op);
void generate_mult_op();
void generate_add_op();

void generate_relational_op(char *op);

void generate_string_mult();

#endif // IFJ_3AC_PATTERNS_H
