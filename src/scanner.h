#ifndef IFJ_SCANNER_H
#define IFJ_SCANNER_H

#include "error.h"
#include "helper.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#define STRING_BLOCK_LEN 50

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
    T_DOT,
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


typedef enum {
    S_NULL,
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
    S_UNDERLINE,
    S_DOUBLE_UNDERLINE,
    S_GLOBAL_ID,
    S_EOL,
    S_MULTI_LINE_LITERAL,
    S_MULTI_LINE_LITERAL_READ,
    S_BLOCK_COMMENT,
    S_BLOCK_COMMENT_2,
    S_BLOCK_COMMENT_3
} tState;

typedef struct Token{
    tType type;
    char *data;
    unsigned int linePos;
    unsigned int colPos;
    struct Token *prevToken;
    struct Token *nextToken;
} *tToken;



int FSM(FILE *file, tToken token);

void scannerError(char currChar, tState state ,unsigned int linePos, unsigned int colPos);

int getToken(FILE *file, tToken *token);

int getTokenList(FILE *file, tToken *firstToken);

void freeToken(tToken *token);

void freeTokenList(tToken *token);

bool isKeyword(tToken token);

char *typeToString(tType type);

void printToken(tToken token);

void printTokenList(tToken token);



#endif // IFJ_SCANNER_H