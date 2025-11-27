/**
 * @file scanner.c
 * 
 * IFJ25 project
 * 
 * Lexical analyzator
 * 
 * @author Jakub Králik <xkralij00>
 */

 
#include "scanner.h"

int FSM(FILE *file, tToken token)
{
    tState state = S_START;
    tState nextState;

    bool active = true;
    static unsigned int linePos = 0;
    static unsigned int colPos = 1;
    static int currChar = -5; // random value < -1 to be sure
    static unsigned int commentNestingLevel = 0; // track nesting level of block comments

    if (currChar == -5)
    {
        currChar = EOL;
    }

    if (file == NULL)
    {
        linePos = 0;
        colPos = 1;
        currChar = -5;
        commentNestingLevel = 0;
        return 0;
    }

    token->type = T_UNKNOWN;
    token->linePos = linePos;
    token->colPos = colPos;

    unsigned int codeStrPos = 0;
    unsigned int codeStrLen = STRING_BLOCK_LEN;
    char *codeStr = safeMalloc(codeStrLen * sizeof(char));

    while (active) {
        nextState = S_NULL;
        if (codeStrPos >= codeStrLen)
        {
            codeStrLen += STRING_BLOCK_LEN;
            codeStr = safeRealloc(codeStr, codeStrLen * sizeof(char));
        }
        codeStr[codeStrPos] = (char)currChar;
        codeStrPos++;

        switch (state)
        {
            case S_START:
                if (currChar == '+') nextState = S_ADD;
                else if (currChar == '-') nextState = S_SUB;
                else if (currChar == '*') nextState = S_MUL;
                else if (currChar == '/') nextState = S_DIV;
                else if (currChar == '>') nextState = S_GREATER;
                else if (currChar == '<') nextState = S_LESS;
                else if (currChar == '=') nextState = S_ASSIGNMENT;
                else if (currChar == '!') nextState = S_NOT;
                else if (currChar == '(') nextState = S_LEFT_PAREN;
                else if (currChar == ')') nextState = S_RIGHT_PAREN;
                else if (currChar == ',') nextState = S_COMMA;
                else if (currChar == ':') nextState = S_COLON;
                else if (currChar == '?') nextState = S_QUESTION;
                else if (currChar == '_') nextState = S_UNDERLINE;
                else if (isalpha(currChar)) nextState = S_ID;
                else if (currChar == '0') nextState = S_INT_0;
                else if (isdigit(currChar)) nextState = S_INT;
                else if (currChar == '"') nextState = S_STRING_START;
                else if (currChar == EOL) nextState = S_EOL;
                else if (currChar == EOF) nextState = S_EOF;
                else if (currChar == '.') nextState = S_DOT;
                else if (currChar == '{') nextState = S_LEFT_BRACE;
                else if (currChar == '}') nextState = S_RIGHT_BRACE;
                else if (isspace(currChar)) nextState = S_SPACE;
                else nextState = S_ERROR;
                break;
            case S_SPACE:
                if (isspace(currChar) && currChar != EOL) nextState = S_SPACE;
                break;
            case S_ADD:
                token->type = T_ADD;
                break;
            case S_SUB:
                token->type = T_SUB;
                break;
            case S_MUL:
                token->type = T_MUL;
                break;
            case S_DIV:
                if (currChar == '/') nextState = S_SINGLE_LINE_COMMENT;
                else if (currChar == '*') 
                {
                    nextState = S_BLOCK_COMMENT;
                    commentNestingLevel = 1;
                }
                else token->type = T_DIV;
                break;
            case S_SINGLE_LINE_COMMENT:
                if (currChar != EOL && currChar != EOF) nextState = S_SINGLE_LINE_COMMENT;
                break;
            case S_BLOCK_COMMENT:
                if (currChar == '*') nextState = S_BLOCK_COMMENT_2;
                else if (currChar == '/') nextState = S_BLOCK_COMMENT_SLASH;
                else if (currChar == EOL) 
                {
                    nextState = S_BLOCK_COMMENT;
                    colPos = 1;
                    linePos++;
                }
                else if (currChar != EOF) nextState = S_BLOCK_COMMENT;
                else nextState = S_ERROR;
                break;
            case S_BLOCK_COMMENT_SLASH:
                if (currChar == '*')
                {
                    commentNestingLevel++;
                    nextState = S_BLOCK_COMMENT;
                }
                else if (currChar == '*') nextState = S_BLOCK_COMMENT_2;
                else if (currChar == '/') nextState = S_BLOCK_COMMENT_SLASH;
                else if (currChar == EOL) 
                {
                    nextState = S_BLOCK_COMMENT;
                    colPos = 1;
                    linePos++;
                }
                else if (currChar != EOF) nextState = S_BLOCK_COMMENT;
                else nextState = S_ERROR;
                break;
            case S_BLOCK_COMMENT_2:
                if (currChar == '/') 
                {
                    commentNestingLevel--;
                    if (commentNestingLevel == 0)
                    {
                        nextState = S_BLOCK_COMMENT_3; // End of all nested comments
                    }
                    else
                    {
                        nextState = S_BLOCK_COMMENT; // Still inside nested comments
                    }
                }
                else if (currChar == '*')
                {
                    nextState = S_BLOCK_COMMENT_2; // Stay in this state for consecutive *
                }
                else if (currChar == '/')
                {
                    nextState = S_BLOCK_COMMENT_SLASH; // Check for potential nested comment start
                }
                else if (currChar == EOL) 
                {
                    nextState = S_BLOCK_COMMENT;
                    colPos = 1;
                    linePos++;
                }
                else if (currChar != EOF) nextState = S_BLOCK_COMMENT;
                else nextState = S_ERROR;
                break;
            case S_COLON:
                token->type = T_COLON;
                break;
            case S_QUESTION:
                token->type = T_QUESTION;
                break;
            case S_GREATER:
                if (currChar == '=') nextState = S_GREATER_EQ;
                else token->type = T_GT;
                break;
            case S_GREATER_EQ:
                token->type = T_GTE;
                break;
            case S_LESS:
                if (currChar == '=') nextState = S_LESS_EQ;
                else token->type = T_LT;
                break;
            case S_LESS_EQ:
                token->type = T_LTE;
                break;
            case S_ASSIGNMENT:
                if (currChar == '=') nextState = S_EQ;
                else token->type = T_ASSIGN;
                break;
            case S_EQ:
                token->type = T_EQL;
                break;
            case S_NOT:
                if (currChar == '=') nextState = S_NOT_EQ;
                else nextState = S_ERROR;
                break;
            case S_NOT_EQ:
                token->type = T_NEQ;
                break;
            case S_LEFT_PAREN:
                token->type = T_LEFT_PAREN;
                break;
            case S_RIGHT_PAREN:
                token->type = T_RIGHT_PAREN;
                break;
            case S_COMMA:
                token->type = T_COMMA;
                break;
            case S_UNDERLINE:
                if (currChar == '_') nextState = S_DOUBLE_UNDERLINE;
                else nextState = S_ERROR;
                break;
            case S_DOUBLE_UNDERLINE:
                if (isalpha(currChar)|| isdigit(currChar) || (currChar == '_')) nextState = S_GLOBAL_ID;
                else nextState = S_ERROR;
                break;
            case S_GLOBAL_ID:
                if (isalpha(currChar)|| isdigit(currChar) || (currChar == '_')) nextState = S_GLOBAL_ID;
                else token->type = T_GLOBAL_ID;
                break;
            case S_ID:
                if (isalpha(currChar) || isdigit(currChar) || currChar == '_') nextState = S_ID;
                else token->type = T_ID;
                break;
            case S_STRING_START:
                if (currChar == '"') nextState = S_STRING_START_2;
                else if (currChar == '\\') nextState = S_STRING_BACKSLASH;
                else if (currChar >= 0x20 && currChar != '"' && currChar != '\\') nextState = S_STRING;
                else nextState = S_ERROR;
                break;
            case S_STRING_START_2:
                if (currChar == '"') nextState = S_MULTI_LINE_LITERAL_CONTENT;
                else token->type = T_STRING;
                break;
            case S_MULTI_LINE_LITERAL_CONTENT:
                if (currChar == '"') nextState = S_MULTI_LINE_LITERAL_END1;
                else if (currChar == EOL) {
                    linePos++;
                    colPos = 1;
                    nextState = S_MULTI_LINE_LITERAL_CONTENT;
                }
                else if (currChar != EOF) nextState = S_MULTI_LINE_LITERAL_CONTENT;
                else nextState = S_ERROR;
                break;
            case S_MULTI_LINE_LITERAL_END1:
                if (currChar == '"') nextState = S_MULTI_LINE_LITERAL_END2;
                else if (currChar == EOL) {
                    linePos++;
                    colPos = 1;
                    nextState = S_MULTI_LINE_LITERAL_CONTENT;
                }
                else if (currChar != EOF) nextState = S_MULTI_LINE_LITERAL_CONTENT;
                else nextState = S_ERROR;
                break;
            case S_MULTI_LINE_LITERAL_END2:
                if (currChar == '"') nextState = S_STRING_READ;
                else if (currChar == EOL) {
                    linePos++;
                    colPos = 1;
                    nextState = S_MULTI_LINE_LITERAL_CONTENT;
                }
                else if (currChar != EOF) nextState = S_MULTI_LINE_LITERAL_CONTENT;
                else nextState = S_ERROR;
                break;
            case S_STRING:
                if (currChar == '"') nextState = S_STRING_READ;
                else if (currChar == '\\') nextState = S_STRING_BACKSLASH;
                else if (currChar >= 0x20 && currChar != '"' && currChar != '\\') nextState = S_STRING;
                else nextState = S_ERROR;
                break;
            case S_STRING_READ:
                token->type = T_STRING;
                break;
            case S_STRING_BACKSLASH:
                if (currChar == 'x') nextState = S_STRING_HEX_START;
                else if (currChar == '"' || currChar == '\\' || currChar == 'n' || currChar == 'r' || currChar == 't') nextState = S_STRING;
                else nextState = S_ERROR;
                break;
            case S_STRING_HEX_START:
                if (isdigit(currChar) || isalpha(currChar)) nextState = S_STRING_HEX_END;
                else nextState = S_ERROR;
                break;
            case S_STRING_HEX_END:
                if (isdigit(currChar) || isalpha(currChar)) nextState = S_STRING;
                else nextState = S_ERROR;
                break;
            case S_INT_0:
                if (currChar == '.') nextState = S_FLOAT_START;
                else if (currChar == 'e' || currChar == 'E') nextState = S_EXP_START;
                else if (currChar == 'x') nextState = S_NUM_HEX_START;
                else token->type = T_INTEGER;
                break;
            case S_NUM_HEX_START:
                if (isdigit(currChar) || (currChar >= 'a' && currChar <= 'f') || (currChar >= 'A' && currChar <= 'F')) nextState = S_NUM_HEX;
                else nextState = S_ERROR;
                break;
            case S_NUM_HEX:
                if (isdigit(currChar) || (currChar >= 'a' && currChar <= 'f') || (currChar >= 'A' && currChar <= 'F')) nextState = S_NUM_HEX;
                else token->type = T_INTEGER;
                break;
            case S_FLOAT_START:
                if (isdigit(currChar)) nextState = S_FLOAT;
                else if (currChar == '.') {
                    ungetc(currChar, file);
                    currChar = '.';
                    state = S_INT;
                    token->type = T_INTEGER;
                    colPos--;
                }
                else nextState = S_ERROR;
                break;
            case S_FLOAT:
                if (isdigit(currChar)) nextState = S_FLOAT;
                else if (currChar == 'e' || currChar == 'E') nextState = S_EXP_START;
                else token->type = T_FLOAT;
                break;
            case S_INT:
                if (isdigit(currChar)) nextState = S_INT;
                else if (currChar == '.') nextState = S_FLOAT_START;
                else if (currChar == 'e' || currChar == 'E') nextState = S_EXP_START;
                else token->type = T_INTEGER;
                break;
            case S_EXP_START:
                if (isdigit(currChar)) nextState = S_EXP;
                else if (currChar == '+' || currChar =='-') nextState = S_EXP_SIGN;
                else nextState = S_ERROR;
                break;
            case S_EXP_SIGN:
                if (isdigit(currChar)) nextState = S_EXP;
                else nextState = S_ERROR;
                break;
            case S_EXP:
                if (isdigit(currChar)) nextState = S_EXP;
                else token->type = T_FLOAT;
                break;
            case S_EOL:
                token->type = T_EOL;
                linePos++;
                colPos = 1;
                break;
            case S_DOT:
                if (currChar == '.') nextState = S_DDOT;
                else token->type = T_DOT;
                break;
            case S_DDOT:
                if (currChar == '.') nextState = S_DDDOT;
                else token->type = T_DDOT;
                break;
            case S_DDDOT:
                token->type = T_DDDOT;
                break;
            case S_LEFT_BRACE:
                token->type = T_LEFT_BRACE;
                break;
            case S_RIGHT_BRACE:
                token->type = T_RIGHT_BRACE;
                break;
            case S_EOF:
                token->type = T_EOF;
                break;
            default:
                break;
        }

        if (nextState == S_NULL || token->type != T_UNKNOWN)
        {
            break;
        } 
        else  if (nextState == S_ERROR)
        {
            scannerError(currChar, state, linePos, colPos);
            active = false;
        }
        

        if (currChar != EOF)
        {
            currChar = fgetc(file);
            colPos++;
        }
        state = nextState;
    }


    switch(state)
    {
        case S_ID:
        case S_GLOBAL_ID:
        case S_INT:
        case S_FLOAT:
        case S_INT_0:
        case S_NUM_HEX:
        case S_STRING_READ:
        case S_STRING_START_2:
        case S_EXP:
            codeStr[codeStrPos-1] = '\0';
            codeStr = safeRealloc(codeStr, codeStrPos);
            token->data = codeStr;
            break;
        default:
            free(codeStr);
            break;
    }

    return nextState == S_ERROR ? 1 : 0;
}







void scannerError(char currChar, tState state, unsigned int linePos, unsigned int colPos) 
{
    fprintf(stderr, "[SCANNER]: Error on line %d:%d - ", linePos, colPos);

    switch(state)
    {
        case S_UNDERLINE:
            fprintf(stderr, "Expected '_' after '_' for declaring global variable, found ");
            break;
        case S_DOUBLE_UNDERLINE:
            fprintf(stderr, "Expected a-z, A-Z, 0-9 or '_' after '__', found ");
            break;
        case S_STRING:
            fprintf(stderr, "Expected printable ASCII char (>=0x20) in string, found ");
            break;
        case S_STRING_BACKSLASH:
            fprintf(stderr, "Expected '\"','n','r','t','\\' or 'x' after '\\' in string, found ");
            break;
        case S_STRING_HEX_START:
        case S_STRING_HEX_END:
            fprintf(stderr, "Expected a-f, A-F or a digit after '\\x' in string, found ");
            break;
        case S_FLOAT_START:
            fprintf(stderr, "Expected a digit after '.' in Num, found ");
            break;
        case S_EXP_START:
            fprintf(stderr, "Expected a digit or +- sign after 'e','E' in Num, found ");
            break;
        case S_EXP_SIGN:
            fprintf(stderr, "Expected a digit after +- sign in Num, found ");
            break;
        case S_NUM_HEX_START:
            fprintf(stderr, "Expected a-f, A-F or a digit after 'x' in Num, found ");
            break;
        case S_MULTI_LINE_LITERAL_CONTENT:
        case S_MULTI_LINE_LITERAL_END1:
        case S_MULTI_LINE_LITERAL_END2:
            fprintf(stderr, "Unterminated multiline string, found ");
            break;
        case S_BLOCK_COMMENT:
        case S_BLOCK_COMMENT_2:
        case S_BLOCK_COMMENT_SLASH:
            fprintf(stderr, "Unterminated block comment, found ");
            break;
        default:
            fprintf(stderr, "Unexpected ");
            break;
    }

    if (currChar >= 0x20)
    {
        fprintf(stderr, "'%c'", currChar);
    } else if (currChar == EOL)
    {
        fprintf(stderr, "EOL");
    } else if (currChar == '\r')
    {
        fprintf(stderr, "'\\r'");
    } else if (currChar == EOF)
    {
        fprintf(stderr, "EOF");
    } else {
        fprintf(stderr, "0x%x", currChar);
    }

    fprintf(stderr, "\n");
}


int getToken(FILE *file, tToken *token)
{
    if (file == NULL || token == NULL) return INTERNAL_ERROR;

    *token = safeMalloc(sizeof(struct Token));
    (*token)->data = NULL;
    (*token)->prevToken = NULL;
    (*token)->nextToken = NULL;

    bool error = false;
    do
    {
        free((*token)->data);
        (*token)->data = NULL;
        error = FSM(file, *token);
    } while (!error && ((*token)->type == T_UNKNOWN || (*token)->linePos == 0));
    
    isKeyword(*token);

    return error ? LEXICAL_ERROR : 0; 
} 


int getTokenList(FILE *file, tToken *firstToken)
{
    tToken lastToken = NULL;

    while (lastToken == NULL || lastToken->type != T_EOF)
    {
        tToken newToken;
        int error = getToken(file, &newToken);
        if (lastToken == NULL)
        {
            *firstToken = newToken;
        } else {
            lastToken->nextToken = newToken;
        }
        newToken->prevToken = lastToken;
        lastToken = newToken;

        if (error != 0)
        {
            freeTokenList(&lastToken);
            *firstToken = NULL;
            return error;
        }
    }

    return 0;
}

void freeToken(tToken *token)
{
    if (token == NULL || *token == NULL)
    {
        return;
    }

    switch((*token)->type)
    {
        case T_STRING:
        case T_INTEGER:
        case T_FLOAT:
        case T_ID:
        case T_GLOBAL_ID:
            free((*token)->data);
            break;
        default:
            break;
    }
}

void freeTokenList(tToken *token)
{
    if (token == NULL || *token == NULL)
    {
        return;
    }

    while ((*token)->nextToken != NULL)
    {
        *token = (*token)->nextToken;
    }

    while (*token != NULL)
    {
        tToken last = (*token)->prevToken;
        freeToken(token);
        *token = last;
    }
}


bool isKeyword(tToken token)
{
    if (token->type != T_ID) return false;

    tType type = token->type;
    if (strcmp(token->data, "class") == 0) type = T_KW_CLASS;
    else if (strcmp(token->data, "if") == 0) type = T_KW_IF;
    else if (strcmp(token->data, "else") == 0) type = T_KW_ELSE;
    else if (strcmp(token->data, "is") == 0) type = T_KW_IS;
    else if (strcmp(token->data, "null") == 0) type = T_KW_NULL_VALUE;
    else if (strcmp(token->data, "return") == 0) type = T_KW_RETURN;
    else if (strcmp(token->data, "var") == 0) type = T_KW_VAR;
    else if (strcmp(token->data, "while") == 0) type = T_KW_WHILE;
    else if (strcmp(token->data, "Ifj") == 0) type = T_KW_IFJ;
    else if (strcmp(token->data, "static") == 0) type = T_KW_STATIC;
    else if (strcmp(token->data, "import") == 0) type = T_KW_IMPORT;
    else if (strcmp(token->data, "for") == 0) type = T_KW_FOR;
    else if (strcmp(token->data, "Num") == 0) type = T_KW_NUM;
    else if (strcmp(token->data, "String") == 0) type = T_KW_STRING;
    else if (strcmp(token->data, "Null") == 0) type = T_KW_NULL_TYPE;
    else if (strcmp(token->data, "in") == 0) type = T_KW_IN;
    else if (strcmp(token->data, "continue") == 0) type = T_KW_CONTINUE;
    else if (strcmp(token->data, "break") == 0) type = T_KW_BREAK;

    if (token->type != type)
    {
        free(token->data);
        token->data = NULL;
        token->type = type;
        return true;
    }

    return false;
}

char *typeToString(tType type)
{
    switch(type)
    {
        case T_UNKNOWN:         return "T_UNKNOWN";
        case T_ID:              return "ID";
        case T_GLOBAL_ID:       return "GLOBAL_ID";
        case T_RIGHT_PAREN:     return "RIGHT_PAREN";
        case T_LEFT_PAREN:      return "LEFT_PAREN";
        case T_RIGHT_BRACE:     return "RIGHT_BRACE";
        case T_LEFT_BRACE:      return "LEFT_BRACE";
        case T_ADD:             return "ADD";
        case T_SUB:             return "SUB";
        case T_MUL:             return "MUL";
        case T_DIV:             return "DIV";
        case T_KW_CLASS:        return "class";
        case T_KW_IF:           return "if";
        case T_KW_ELSE:         return "else";
        case T_KW_IS:           return "is";
        case T_KW_NULL_VALUE:   return "null";
        case T_KW_RETURN:       return "return";
        case T_KW_VAR:          return "var";
        case T_KW_WHILE:        return "while";
        case T_KW_IFJ:          return "Ifj";
        case T_KW_STATIC:       return "static";
        case T_KW_IMPORT:       return "import";
        case T_KW_FOR:          return "for";
        case T_KW_NUM:          return "Num";
        case T_KW_STRING:       return "String";
        case T_KW_NULL_TYPE:    return "Null";
        case T_KW_BREAK:        return "return";
        case T_KW_CONTINUE:     return "continue";
        case T_KW_IN:           return "IN";
        case T_DOT:             return "DOT";
        case T_DDOT:            return "DDOT";
        case T_DDDOT:           return "DDDOT";
        case T_EOL:             return "EOL";
        case T_EOF:             return "EOF";
        case T_EQL:             return "EQL";
        case T_NEQ:             return "NEQ";
        case T_LT:              return "LT";
        case T_GT:              return "GT";
        case T_LTE:             return "LTE";
        case T_GTE:             return "GTE";
        case T_ASSIGN:          return "ASSIGN";
        case T_INTEGER:         return "INTEGER";
        case T_FLOAT:           return "FLOAT";
        case T_STRING:          return "STRING";
        case T_NULL:            return "NULL";
        case T_COMMA:           return "COMMA";
        case T_COLON:           return "COLON";
        case T_QUESTION:        return "QUESTION";
        default:                return "UNKNOWN";
    }
}

void printToken(tToken token)
{
    if (token == NULL)
    {
        printf("Token is NULL\n");
        return;
    }

    printf("$%d:%d\t", token->linePos, token->colPos);
    printf("%s",typeToString(token->type));

    switch(token->type)
    {
        case T_STRING:
        case T_INTEGER:
        case T_FLOAT:
        case T_ID:
        case T_GLOBAL_ID:
            printf("(%s)", token->data);
            break;
        default:
            break;
    }
    printf("\n");
}


void printTokenList(tToken token)
{
    if (token == NULL)
    {
        printf("Token is NULL\n");
        return;
    }

    while (token->prevToken != NULL)
    {
        token = token->prevToken;
    }

    while (token != NULL) // token->nextToken != NULL <--> if you don't want to print EOF token
    {
        printToken(token);
        token = token->nextToken;
    }
}

