#include "semantic.h"
#include "symtable.h"
#include "error.h"
#include <stdio.h>
#include "symstack.h"
#include "parser.h"

char* transform_to_data_type(int type) { 
    switch (type) { 
        case 0: 
            return "NUM"; 
        case 1: 
            return "STRING"; 
        case 2: 
            return "NULL"; 
        default: 
            return "UNDEF"; 
    }
}

void semantic_check_if_func_exists(tSymbolData *funcData, const char *name) 
{ 
    if (funcData == NULL || funcData->kind != SYM_FUNC) 
    { 
        fprintf(stderr, "[SEMANTIC] Error: builtin function '%s' not found\n", name); 
        exit(UNDEFINED_FUN_ERROR); 
    } 
}

void semantic_check_argument_count(tSymbolData *funcData, int argCount, const char *name) 
{ 
    if (argCount != funcData->paramCount)
    {
        fprintf(stderr, "[SEMANTIC] Error: builtin '%s' expects %d arguments, got %d\n",
                name, funcData->paramCount, argCount);
        exit(WRONG_ARGUMENT_COUNT_ERROR);
    }
}

void semantic_check_ifj_call(tSymbolData *funcData, tDataType *argTypes, int argCount, const char *name) 
{ 
    semantic_check_if_func_exists(funcData, name);
    semantic_check_argument_count(funcData, argCount, name);

    for (int i = 0; i < argCount; i++) 
    {   
        tDataType expected = funcData->paramTypes[i];
        tDataType given = argTypes[i];

        printf("IFJ CALL ARG %d: expected %d, got %d\n", i + 1, expected, given);
        if (expected != 3 && given != 3 && expected != given) 
        { 
            fprintf(stderr, "[SEMANTIC] Type mismatch in function '%s' argument %d: expected %s, got %s\n", name, i + 1, transform_to_data_type(expected), transform_to_data_type(given)); 
            exit(WRONG_ARGUMENT_COUNT_ERROR); 
        }
    }
}


extern tSymTable *global_symtable;
void semantic_define_variable(tSymTableStack *stack, const char *variable_name, bool isGlobal) { 
    tSymbolData data = {0};
    data.kind = SYM_VAR;
    data.dataType = TYPE_UNDEF;
    data.index = -1;

    bool success = (isGlobal) ? symtable_insert(global_symtable, variable_name, data) : symtable_insert(symtable_stack_top(stack), variable_name, data);

    if (!success)
    {
        fprintf(stderr, "[SEMANTIC] Error: variable '%s' redefined\n", variable_name);
        exit(REDEFINITION_FUN_ERROR);
    }
    free((void *)variable_name);
}