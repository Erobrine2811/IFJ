/**
 * @file scanner.h
 * 
 * IFJ25 project
 * 
 * Lexical analyzator
 * 
 * @author Jakub Králik <xkralij00>
 */



#ifndef IFJ_SCANNER_H
#define IFJ_SCANNER_H

#include "error.h"
#include "helper.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>



/**
 * Length of string blocks for dynamic string allocation
 */
#define STRING_BLOCK_LEN 50



/**
 * Enumeration of all possible token types
 */
typedef enum {
    T_UNKNOWN,
    T_ID,
    T_GLOBAL_ID,
    T_RIGHT_PAREN,
    T_LEFT_PAREN,
    T_RIGHT_BRACE,
    T_LEFT_BRACE,
    T_ADD,
    T_SUB,
    T_MUL,
    T_DIV,
    T_KW_CLASS,
    T_KW_IF,
    T_KW_ELSE,
    T_KW_IS,
    T_KW_NULL_VALUE,
    T_KW_RETURN,
    T_KW_VAR,
    T_KW_WHILE,
    T_KW_IFJ,
    T_KW_STATIC,
    T_KW_IMPORT,
    T_KW_FOR,
    T_KW_NUM,
    T_KW_STRING,
    T_KW_NULL_TYPE,
    T_KW_IN,
    T_KW_CONTINUE,
    T_KW_BREAK,
    T_COLON,
    T_DOT,
    T_DDOT,
    T_DDDOT,
    T_EOL,
    T_EOF,
    T_EQL,
    T_NEQ,
    T_LT,
    T_GT,
    T_LTE,
    T_GTE,
    T_ASSIGN,
    T_INTEGER,
    T_FLOAT,
    T_STRING,
    T_NULL,
    T_COMMA
} tType;



/**
 * Enumeration of all possible states of the finite state machine
 */
typedef enum {
    S_NULL,
    S_SPACE,
    S_ERROR,
    S_START,
    S_ADD,
    S_SUB,
    S_MUL,
    S_DIV,
    S_NOT,
    S_SINGLE_LINE_COMMENT,
    S_GREATER,
    S_GREATER_EQ,
    S_LESS,
    S_LESS_EQ,
    S_ASSIGNMENT,
    S_EQ,
    S_NOT_EQ,
    S_LEFT_PAREN,
    S_RIGHT_PAREN,
    S_COMMA,
    S_ID,
    S_STRING,
    S_STRING_READ,
    S_STRING_BACKSLASH,
    S_STRING_HEX_START,
    S_STRING_HEX_END,
    S_INT,
    S_EXP_START,
    S_EXP_SIGN,
    S_EXP,
    S_FLOAT_START,
    S_FLOAT,
    S_INT_0,
    S_NUM_HEX_START,
    S_NUM_HEX,
    S_EOF,
    S_LEFT_BRACE,
    S_RIGHT_BRACE,
    S_DOT,
    S_DDOT,
    S_DDDOT,
    S_UNDERLINE,
    S_DOUBLE_UNDERLINE,
    S_GLOBAL_ID,
    S_COLON,
    S_EOL,
    S_STRING_START,
    S_STRING_START_2,
    S_MULTI_LINE_LITERAL_CONTENT,
    S_MULTI_LINE_LITERAL_END1,
    S_MULTI_LINE_LITERAL_END2,
    S_BLOCK_COMMENT,
    S_BLOCK_COMMENT_2,
    S_BLOCK_COMMENT_3,
    S_BLOCK_COMMENT_SLASH
} tState;



/**
 * Structure representing a token
 * can be linked to form a list of tokens
 */
typedef struct Token{
    tType type;
    char *data;
    unsigned int linePos;
    unsigned int colPos;
    struct Token *prevToken;
    struct Token *nextToken;
} *tToken;



/**
 * Finite State Machine for lexical analysis
 * 
 * @param file Input file to read from
 * @param token Pointer to token structure to fill
 * @return 0 on success, LEXICAL_ERROR on lexical error, INTERNAL_ERROR on internal error
 */
int FSM(FILE *file, tToken token);



/**
 * Function to print scanner error messages
 * 
 * @param currChar Current character causing the error
 * @param state Current state of the FSM
 * @param linePos Current line position in the input file
 * @param colPos Current column position in the input file
 */
void scannerError(char currChar, tState state ,unsigned int linePos, unsigned int colPos);



/**
 * Function to get the next token from the input file
 * 
 * @param file Input file to read from
 * @param token Pointer to token structure to fill
 * @return 0 on success, LEXICAL_ERROR on lexical error, INTERNAL_ERROR on internal error
 */
int getToken(FILE *file, tToken *token);



/**
 * Function to get the list of all tokens from the input file
 * 
 * @param file Input file to read from
 * @param firstToken Pointer to the first token in the list
 * @return 0 on success, LEXICAL_ERROR on lexical error, INTERNAL_ERROR on internal error
 */
int getTokenList(FILE *file, tToken *firstToken);



/**
 * Function to free a token
 * 
 * @param token Pointer to token to free
 */
void freeToken(tToken *token);



/**
 * Function to free a list of tokens
 * 
 * @param token Pointer to the first token in the list
 */
void freeTokenList(tToken *token);



/**
 * Function to check if a token is a keyword
 * If it is, the token type is changed to the corresponding keyword type
 * 
 * @param token Token to check
 * @return true if the token is a keyword, false otherwise
 */
bool isKeyword(tToken token);



/**
 * Function to convert a token type to a string
 * 
 * @param type Token type to convert
 * @return String representation of the token type
 */
char *typeToString(tType type);



/**
 * Function to print a token
 * 
 * @param token Token to print
 */
void printToken(tToken token);



/**
 * Function to print a list of tokens
 * 
 * @param token Pointer to the first token in the list
 */
void printTokenList(tToken token);



#endif // IFJ_SCANNER_H