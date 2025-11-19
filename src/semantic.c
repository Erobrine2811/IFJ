#include "semantic.h"
#include "symtable.h"
#include "error.h"
#include <stdio.h>
#include "symstack.h"
#include "parser.h"

char* transform_to_data_type(int type) { 
    switch (type) { 
        case TYPE_INT:
            return "INT";
        case TYPE_FLOAT:
            return "FLOAT";
        case TYPE_NUM: 
            return "NUM"; 
        case TYPE_STRING: 
            return "STRING"; 
        case TYPE_NULL: 
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

      
        if (expected != TYPE_UNDEF && given != TYPE_UNDEF && expected != given) 
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
}

tDataType semantic_check_operation(char* op, tDataType left, tDataType right) {
    if (left == TYPE_UNDEF || right == TYPE_UNDEF) {
        return TYPE_UNDEF;
    }

    if (left == TYPE_NULL || right == TYPE_NULL) {
        fprintf(stderr, "[SEMANTIC] Type error in '%s' operation: operand cannot be null\n", op);
        exit(TYPE_COMPATIBILITY_ERROR);
    }

    bool left_is_num = (left == TYPE_INT || left == TYPE_FLOAT || left == TYPE_NUM);
    bool right_is_num = (right == TYPE_INT || right == TYPE_FLOAT || right == TYPE_NUM);

    if (strcmp(op, "+") == 0) {
        if (left_is_num && right_is_num) {
            if (left == TYPE_FLOAT || right == TYPE_FLOAT) return TYPE_FLOAT;
            if (left == TYPE_NUM || right == TYPE_NUM) return TYPE_NUM;
            return TYPE_INT;
        }
        if (left == TYPE_STRING && right == TYPE_STRING) return TYPE_STRING;
        fprintf(stderr, "[SEMANTIC] Type error in '+' operation: cannot add %s and %s\n", transform_to_data_type(left), transform_to_data_type(right));
        exit(TYPE_COMPATIBILITY_ERROR);
    }
        if (left_is_num && right_is_num) {
            if (strcmp(op, "/") == 0) {
                if (left == TYPE_NUM || right == TYPE_NUM) return TYPE_NUM;
                return TYPE_FLOAT;
            }
        fprintf(stderr, "[SEMANTIC] Type error in '%s' operation: incompatible types %s and %s\n", op, transform_to_data_type(left), transform_to_data_type(right));
        exit(TYPE_COMPATIBILITY_ERROR);
    }
    if (strcmp(op, "*") == 0) {
        if (left_is_num && right_is_num) {
            if (left == TYPE_FLOAT || right == TYPE_FLOAT) return TYPE_FLOAT;
            if (left == TYPE_NUM || right == TYPE_NUM) return TYPE_NUM;
            return TYPE_INT;
        }
        if ((left == TYPE_STRING && right == TYPE_INT) || (left == TYPE_INT && right == TYPE_STRING)) {
            return TYPE_STRING;
        }
        fprintf(stderr, "[SEMANTIC] Type error in '*' operation: incompatible types %s and %s\n", transform_to_data_type(left), transform_to_data_type(right));
        exit(TYPE_COMPATIBILITY_ERROR);
    }

    return TYPE_UNDEF;
}
