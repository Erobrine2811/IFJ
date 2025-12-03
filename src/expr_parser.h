/**
 * @file expr_parser.h
 *
 * IFJ25 project
 *
 * Expression parser using precedence analysis
 *
 * @author Lukáš Denkócy <xdenkol00>
 */

#ifndef IFJ_EXPR_PARSER_H
#define IFJ_EXPR_PARSER_H

#include "3AC.h"
#include "error.h"
#include "scanner.h"
#include "semantic.h"
#include "symstack.h"
#include "symtable.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Enumeration of symbols for the precedence table.
 */
typedef enum
{
    E_MUL_DIV,
    E_PLUS_MINUS,
    E_REL,
    E_EQ_NEQ,
    E_IS,
    E_TYPE,
    E_LPAREN,
    E_RPAREN,
    E_ID,
    E_LITERAL,
    E_FUNC,
    E_DOLLAR
} tSymbol;

/**
 * Enumeration of precedence relations.
 */
typedef enum
{
    PREC_LESS = '<',
    PREC_GREATER = '>',
    PREC_EQUAL = '=',
    PREC_ERROR = 'E'
} tPrec;

/**
 * Structure representing a node on the expression parser's stack.
 */
typedef struct ExprStackNode
{
    tSymbol symbol;
    bool isTerminal;
    char *value;
    tDataType dataType;
    struct ExprStackNode *next;
} tExprStackNode;

/**
 * Structure representing the expression parser's stack.
 */
typedef struct
{
    tExprStackNode *top;
} tExprStack;

/**
 * Parses a single expression using precedence parsing rules.
 *
 * @param file The input file stream.
 * @param currentToken The current token from the scanner, will be updated.
 * @param stack The symbol table stack for context.
 * @return The resulting data type of the expression.
 */
tDataType parse_expression(FILE *file, tToken *currentToken, tSymTableStack *stack);

/**
 * Processes a raw string literal, handling escape sequences.
 *
 * @param rawData The raw string data from the token.
 * @return A new dynamically allocated string with escape sequences processed.
 */
char *process_string_literal(const char *rawData);

/**
 * Creates a 3-address code operand from a given token.
 *
 * @param token The token to convert.
 * @param symStack The symbol table stack for context.
 * @return A pointer to a new tOperand structure.
 */
tOperand *create_operand_from_token(tToken token, tSymTableStack *symStack);

/**
 * Determines the data type of a token by checking the symbol table.
 *
 * @param token The token to analyze.
 * @param symStack The symbol table stack for context.
 * @return The data type of the token.
 */
tDataType get_data_type_from_token(tToken token, tSymTableStack *symStack);

/**
 * Maps a token to its corresponding symbol for the precedence table.
 *
 * @param token The token to map.
 * @param file The input file stream (for lookahead in case of function calls).
 * @return The corresponding tSymbol for the precedence table.
 */
tSymbol get_precedence_type(tToken token, FILE *file);

/**
 * Pushes a new symbol onto the expression stack.
 *
 * @param stack The expression stack.
 * @param sym The symbol to push.
 * @param isTerminal True if the symbol is a terminal.
 */
void expr_push(tExprStack *stack, tSymbol sym, bool isTerminal);

/**
 * Pops the top symbol from the expression stack.
 *
 * @param stack The expression stack.
 */
void expr_pop(tExprStack *stack);

/**
 * Finds the first terminal symbol from the top of the expression stack.
 *
 * @param stack The expression stack.
 * @return A pointer to the top-most terminal node on the stack.
 */
tExprStackNode *expr_top_terminal(tExprStack *stack);

/**
 * Pops from the expression stack until a left parenthesis (marker) is found.
 *
 * @param stack The expression stack.
 */
void expr_pop_until_marker(tExprStack *stack);

/**
 * Checks if a token signifies the end of an expression.
 *
 * @param token The token to check.
 * @return 1 if it's the end of an expression, 0 otherwise.
 */
int is_token_expr_end(tToken *token);

/**
 * Performs a reduction on the expression stack based on grammar rules.
 *
 * @param stack The expression stack.
 * @return 1 on success, otherwise 0.
 */
int reduce_expr(tExprStack *stack);

#endif // IFJ_EXPR_PARSER_H
