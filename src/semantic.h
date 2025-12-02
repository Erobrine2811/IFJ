#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "3AC.h"
#include "symstack.h"
#include "symtable.h"
#include <stdbool.h>

/**
 * Function to check semantic argument types and argument count of Ifj buildin function
 *
 * @param funcData Function symbol data
 * @param argTypes Argument types passed to the function
 * @param argCount Number of arguments passed to the function
 * @param name Name of the function being called
 */
void semantic_check_ifj_call(tSymbolData *funcData, tDataType *argTypes, int argCount,
                             const char *name);

/**
 * Function to check semantic argument types and argument count of Ifj buildin function
 *
 * @param funcData Function symbol data
 * @param argTypes Argument types passed to the function
 * @param argCount Number of arguments passed to the function
 * @param name Name of the function being called
 */
void semantic_define_variable(tSymTableStack *stack, const char *variable_name, bool isGlobal);

/**
 * Function to check semantic argument count of function call
 *
 * @param funcData Function symbol data
 * @param argCount Number of arguments passed to the function
 * @param name Name of the function being called
 */
void semantic_check_argument_count(tSymbolData *funcData, int argCount, const char *name);

/**
 * Function to check semantic rules for binary operations.
 *
 * @param op The operator string (e.g., "+", "*").
 * @param left The data type of the left operand.
 * @param right The data type of the right operand.
 * @return The data type of the result of the operation.
 */
tDataType semantic_check_literal_operation(char *op, tDataType left, tDataType right);

#endif
