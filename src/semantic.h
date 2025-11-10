#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "symtable.h"
#include "symstack.h"
#include <stdbool.h>

/**
 * Function to check semantic argument types and argument count of Ifj buildin function
 * 
 * @param funcData Function symbol data
 * @param argTypes Argument types passed to the function
 * @param argCount Number of arguments passed to the function
 * @param name Name of the function being called
 */
void semantic_check_ifj_call(tSymbolData *funcData, tDataType *argTypes, int argCount, const char *name);



/**
 * Function to check semantic argument types and argument count of Ifj buildin function
 * 
 * @param funcData Function symbol data
 * @param argTypes Argument types passed to the function
 * @param argCount Number of arguments passed to the function
 * @param name Name of the function being called
 */
void semantic_define_variable(tSymTableStack *stack, const char *variable_name, bool isGlobal);

#endif