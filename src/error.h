/**
 * @file error.h
 * 
 * IFJ25 project
 * 
 * Basic error exit codes
 * 
 * @author Jakub Králik <xkralij00>
 */



#ifndef IFJ_ERROR_H
#define IFJ_ERROR_H

#define LEXICAL_ERROR 1
#define SYNTAX_ERROR 2 
#define UNDEFINED_FUN_ERROR 3
#define REDEFINITION_FUN_ERROR 4 
#define WRONG_ARGUMENT_COUNT_ERROR 5 
#define TYPE_COMPATIBILITY_ERROR 6
#define OTHER_SEMANTIC_ERROR 10
#define RUNTIME_PARAM_TYPE_ERROR 25
#define RUNTIME_TYPE_COMPATIBILITY_ERROR 26
#define INTERNAL_ERROR 99


#endif // IFJ_ERROR_H  