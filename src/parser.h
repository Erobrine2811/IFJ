/**
 * @file parser.h
 *
 * IFJ25 project
 *
 * Syntactic and semantic analyzer
 *
 * @author Lukáš Denkócy <xdenkol00>
 */

#ifndef IFJ_PARSER_H
#define IFJ_PARSER_H

#include "error.h"
#include "expr_parser.h"
#include "helper.h"
#include "scanner.h"
#include "symstack.h"
#include "symtable.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Pointer to the global symbol table for the program.
 */
extern tSymTable *global_symtable;

/**
 * Structure for a built-in function definition.
 */
typedef struct
{
    char *name;
    tDataType returnType;
    int paramCount;
    tDataType params[3];
} tBuiltinDef;

/**
 * Main entry point for the parser. Parses the entire program from a file.
 *
 * @param file The input file stream to parse.
 * @return An error code, 0 on success.
 */
int parse_program(FILE *file);

/**
 * Looks at the next token in the stream without consuming it.
 *
 * @param file The input file stream.
 * @return The next token.
 */
tToken peek_token(FILE *file);

/**
 * Consumes the current token and fetches the next one from the stream.
 *
 * @param file The input file stream.
 * @param currentToken Pointer to the token to be updated with the next token.
 */
void get_next_token(FILE *file, tToken *currentToken);

/**
 * Parses the program prolog (e.g., 'import "ifj25" for Ifj').
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 */
void parse_prolog(FILE *file, tToken *currentToken);

/**
 * Parses the main class definition block.
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 */
void parse_class_def(FILE *file, tToken *currentToken, tSymTableStack *stack);

/**
 * Inserts all built-in functions into the global symbol table.
 */
void insert_builtin_functions();

/**
 * Parses a list of function definitions within the class body.
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 */
void parse_func_list(FILE *file, tToken *currentToken, tSymTableStack *stack);

/**
 * Parses a single function declaration.
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 */
void parse_function_declaration(FILE *file, tToken *currentToken, tSymTableStack *stack);

/**
 * Parses a getter function (a function with no parameters).
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 * @param funcName The name of the function.
 */
void parse_getter(FILE *file, tToken *currentToken, tSymTableStack *stack, char *funcName);

/**
 * Parses a setter function (a function with one parameter).
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 * @param funcName The name of the function.
 */
void parse_setter(FILE *file, tToken *currentToken, tSymTableStack *stack, char *funcName);

/**
 * Parses a single statement.
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 */
void parse_statement(FILE *file, tToken *currentToken, tSymTableStack *stack);

/**
 * Checks if a symbol table node (function) has been defined (has a body).
 *
 * @param node The symbol table node to check.
 */
void check_node_defined(tSymNode *node);

/**
 * Iterates through the symbol table to find any functions that were declared but not defined.
 */
void check_undefined_functions();

/**
 * Parses an if-else statement.
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 */
void parse_if_statement(FILE *file, tToken *currentToken, tSymTableStack *stack);

/**
 * Parses a while loop statement.
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 */
void parse_while_statement(FILE *file, tToken *currentToken, tSymTableStack *stack);

/**
 * Parses a list of parameters in a function declaration.
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 * @param paramNames A pointer to an array of strings to store parameter names.
 * @return The number of parameters found.
 */
int parse_parameter_list(FILE *file, tToken *currentToken, tSymTableStack *stack,
                         char ***paramNames);
/**
 * Parses a variable declaration statement.
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 */
void parse_variable_declaration(FILE *file, tToken *currentToken, tSymTableStack *stack);

/**
 * Parses an assignment statement.
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 */
void parse_assignment_statement(FILE *file, tToken *currentToken, tSymTableStack *stack);

/**
 * Parses a block of statements enclosed in curly braces.
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 * @param isFunctionBody True if the block is a function body, false otherwise.
 */
void parse_block(FILE *file, tToken *currentToken, tSymTableStack *stack, bool isFunctionBody);

/**
 * Parses a function call.
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 * @param isStatement True if the function call is a standalone statement.
 */
void parse_function_call(FILE *file, tToken *currentToken, tSymTableStack *stack, bool isStatement);

/**
 * Parses a call to a built-in 'ifj' function.
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner.
 * @param stack The symbol table stack.
 * @param isStatement True if the function call is a standalone statement.
 * @return The data type of the return value of the called function.
 */
tDataType parse_ifj_call(FILE *file, tToken *currentToken, tSymTableStack *stack, bool isStatement);

/**
 * Expects a token of a specific type and consumes it, otherwise exits with an error.
 *
 * @param type The expected token type.
 * @param currentToken The current token from the scanner.
 * @param file The input file stream.
 * @param checkValue If true, also checks the token's string value.
 * @param value The expected string value if checkValue is true.
 */
void expect_and_consume(tType type, tToken *currentToken, FILE *file, bool checkValue,
                        const char *value);
/**
 * Skips an end-of-line token if it is the current token.
 *
 * @param currentToken The current token from the scanner.
 * @param file The input file stream.
 */
void skip_optional_eol(tToken *currentToken, FILE *file);

/**
 * Searches for a symbol by key through the entire symbol table stack.
 *
 * @param stack The symbol table stack to search in.
 * @param key The key of the symbol to find.
 * @return A pointer to the symbol's data if found, otherwise NULL.
 */

#endif // IFJ_PARSER_H
