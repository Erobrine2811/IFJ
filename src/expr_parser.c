#include "3AC.h"
#include "3AC_patterns.h"
#include "expr_parser.h"
#include "parser.h"
#include "semantic.h"
#include "symtable.h"
#include <stdio.h>

// clang-format off
static tPrec precedence_table[12][12] = {
/*                MUL_DIV  PLUS_MINUS   REL     EQ_NEQ   IS     TYPE    LPAREN  RPAREN     ID   LITERAL   FUNC    DOLLAR */
/* MUL_DIV */    { '>',     '>',        '>',    '>',     '>',   'E',     '<',    '>',    '<',    '<',     '<',     '>'  },
/* PLUS_MINUS */ { '<',     '>',        '>',    '>',     '>',   'E',     '<',    '>',    '<',    '<',     '<',     '>'  },
/* REL */        { '<',     '<',        '>',    '>',     '>',   'E',     '<',    '>',    '<',    '<',     '<',     '>'  },
/* EQ_NEQ */     { '<',     '<',        '<',    '>',     '>',   'E',     '<',    '>',    '<',    '<',     '<',     '>'  },
/* IS */         { '<',     '<',        '<',    '<',     'E',   '<',     '<',    '>',    '<',    '<',     '<',     '>'  },
/* TYPE */       { '>',     '>',        '>',    '>',     '>',   '>',     'E',    '>',    'E',    'E',     'E',     '>'  },
/* LPAREN */     { '<',     '<',        '<',    '<',     '<',   '<',     '<',    '=',    '<',    '<',     '<',     'E'  },
/* RPAREN */     { '>',     '>',        '>',    '>',     '>',   '>',     'E',    '>',    'E',    'E',     'E',     '>'  },
/* ID */         { '>',     '>',        '>',    '>',     '>',   'E',     'E',    '>',    'E',    'E',     'E',     '>'  },
/* LITERAL */    { '>',     '>',        '>',    '>',     '>',   'E',     'E',    '>',    'E',    'E',     'E',     '>'  },
/* FUNC */       { '>',     '>',        '>',    '>',     '>',   'E',     'E',    '>',    'E',    'E',     'E',     '>'  },
/* DOLLAR */     { '<',     '<',        '<',    '<',     '<',   '<',     '<',    'E',    '<',    '<',     '<',     'E'  },
};
// clang-format on

static char *process_string_literal(const char *rawData)
{
    int len = strlen(rawData);

    if (strncmp(rawData, "\"\"\"", 3) == 0)
    {
        // Multiline string
        if (len < 6)
        {
            char *s = safeMalloc(1);
            s[0] = '\0';
            return s;
        }

        const char *contentStart = rawData + 3;
        const char *contentEnd = rawData + len - 3;

        // Trim leading whitespace and a single newline
        const char *p = contentStart;
        bool onlyWhitespaceBeforeNewline = true;
        while (p < contentEnd && *p != '\n')
        {
            if (!isspace((unsigned char)*p))
            {
                onlyWhitespaceBeforeNewline = false;
                break;
            }
            p++;
        }
        if (onlyWhitespaceBeforeNewline && p < contentEnd && *p == '\n')
        {
            contentStart = p + 1;
        }

        // Trim trailing newline and whitespace
        p = contentEnd - 1;
        bool onlyWhitespaceAfterNewline = true;
        while (p >= contentStart && *p != '\n')
        {
            if (!isspace((unsigned char)*p))
            {
                onlyWhitespaceAfterNewline = false;
                break;
            }
            p--;
        }
        if (onlyWhitespaceAfterNewline && p >= contentStart && *p == '\n')
        {
            contentEnd = p;
        }

        if (contentStart >= contentEnd)
        {
            char *emptyStr = safeMalloc(1);
            emptyStr[0] = '\0';
            return emptyStr;
        }

        size_t newLen = contentEnd - contentStart;
        char *trimmedStr = safeMalloc(newLen + 1);
        strncpy(trimmedStr, contentStart, newLen);
        trimmedStr[newLen] = '\0';

        char *finalStr = safeMalloc(newLen + 1);
        int j = 0;
        for (int i = 0; i < newLen; i++)
        {
            if (trimmedStr[i] == '\\')
            {
                i++;
                if (i >= newLen)
                {
                    break;
                }
                switch (trimmedStr[i])
                {
                    case 'n':
                        finalStr[j++] = '\n';
                        break;
                    case 'r':
                        finalStr[j++] = '\r';
                        break;
                    case 't':
                        finalStr[j++] = '\t';
                        break;
                    case '"':
                        finalStr[j++] = '"';
                        break;
                    case '\\':
                        finalStr[j++] = '\\';
                        break;
                    case 'x':
                        if (i + 2 < newLen && isxdigit(trimmedStr[i + 1]) &&
                            isxdigit(trimmedStr[i + 2]))
                        {
                            char hex[3] = {trimmedStr[i + 1], trimmedStr[i + 2], '\0'};
                            finalStr[j++] = (char)strtol(hex, NULL, 16);
                            i += 2;
                        }
                        else
                        {
                            finalStr[j++] = 'x';
                        }
                        break;
                    default:
                        finalStr[j++] = '\\';
                        finalStr[j++] = trimmedStr[i];
                        break;
                }
            }
            else
            {
                finalStr[j++] = trimmedStr[i];
            }
        }
        finalStr[j] = '\0';
        free(trimmedStr);
        return safeRealloc(finalStr, j + 1);
    }
    else
    { // Regular string
        if (len < 2)
        {
            char *s = safeMalloc(1);
            s[0] = '\0';
            return s;
        }

        char *unescapedStr = safeMalloc(len);
        int j = 0;
        for (int i = 1; i < len - 1; i++)
        {
            if (rawData[i] == '\\')
            {
                i++;
                if (i >= len - 1)
                {
                    break;
                }
                switch (rawData[i])
                {
                    case 'n':
                        unescapedStr[j++] = '\n';
                        break;
                    case 'r':
                        unescapedStr[j++] = '\r';
                        break;
                    case 't':
                        unescapedStr[j++] = '\t';
                        break;
                    case '"':
                        unescapedStr[j++] = '"';
                        break;
                    case '\\':
                        unescapedStr[j++] = '\\';
                        break;
                    case 'x':
                        if (i + 2 < len - 1 && isxdigit(rawData[i + 1]) && isxdigit(rawData[i + 2]))
                        {
                            char hex[3] = {rawData[i + 1], rawData[i + 2], '\0'};
                            unescapedStr[j++] = (char)strtol(hex, NULL, 16);
                            i += 2;
                        }
                        else
                        {
                            unescapedStr[j++] = 'x';
                        }
                        break;
                    default:
                        unescapedStr[j++] = '\\';
                        unescapedStr[j++] = rawData[i];
                        break;
                }
            }
            else
            {
                unescapedStr[j++] = rawData[i];
            }
        }
        unescapedStr[j] = '\0';
        return safeRealloc(unescapedStr, j + 1);
    }
}

// Helper to create an Operand from a token
static Operand *create_operand_from_token(tToken token, tSymTableStack *symStack)
{
    if (!token)
        return NULL;

    Operand *op;
    char *dataCopy = NULL;

    if (token->data)
    {
        dataCopy = safeMalloc(strlen(token->data) + 1);
        strcpy(dataCopy, token->data);
    }
    else if (token->type == T_KW_NULL_VALUE)
    {
        op = create_operand_from_constant_nil();
        return op;
    }
    else
    {
        return NULL;
    }

    switch (token->type)
    {
        case T_INTEGER:
            op = create_operand_from_constant_int(atoi(dataCopy));
            break;
        case T_FLOAT:
            op = create_operand_from_constant_float(atof(dataCopy));
            break;
        case T_STRING:
        {
            op = create_operand_from_constant_string(process_string_literal(dataCopy));
            break;
        }
        case T_KW_NULL_VALUE:
            op = create_operand_from_constant_nil();
            break;
        case T_ID:
        {
            int getterKeyLen = strlen("getter:") + strlen(dataCopy) + 3;
            char *getterKey = safeMalloc(getterKeyLen);
            sprintf(getterKey, "getter:%s@0", dataCopy);

            tSymbolData *getterData = symtable_find(global_symtable, getterKey);

            if (getterData)
            {
                emit(OP_CREATEFRAME, NULL, NULL, NULL, &threeACcode);
                emit(OP_PUSHFRAME, NULL, NULL, NULL, &threeACcode);

                char mangledName[256];
                sprintf(mangledName, "%s$0%%getter", dataCopy);
                Operand *callLabel = create_operand_from_label(mangledName);
                emit(OP_CALL, callLabel, NULL, NULL, &threeACcode);

                emit(OP_POPFRAME, NULL, NULL, NULL, &threeACcode);
                op = create_operand_from_tf_variable("%retval");

                return op;
            }

            tSymbolData *data = find_data_in_stack(symStack, dataCopy);

            // Treat as getter not yet defined
            if (!data)
            {
                data = safeMalloc(sizeof(tSymbolData));
                data->kind = SYM_FUNC;
                data->dataType = TYPE_UNDEF;
                data->returnType = TYPE_UNDEF;
                data->index = -1;
                data->defined = false;
                data->paramCount = 0;
                data->paramNames = NULL;
                data->unique_name = NULL;

                if (!symtable_insert(global_symtable, getterKey, *data))
                {
                    fprintf(stderr, "[PARSER] Error inserting forward decl for '%s'\n", getterKey);
                    exit(INTERNAL_ERROR);
                }

                emit(OP_CREATEFRAME, NULL, NULL, NULL, &threeACcode);
                emit(OP_PUSHFRAME, NULL, NULL, NULL, &threeACcode);

                char mangledName[256];
                sprintf(mangledName, "%s$0%%getter", dataCopy);
                Operand *callLabel = create_operand_from_label(mangledName);
                emit(OP_CALL, callLabel, NULL, NULL, &threeACcode);

                emit(OP_POPFRAME, NULL, NULL, NULL, &threeACcode);
                op = create_operand_from_tf_variable("%retval");
                free(getterKey);
                free(data);
                break;
            }

            op = create_operand_from_variable(data->unique_name, false);

            break;
        }
        case T_GLOBAL_ID:
        {
            tSymbolData *data = find_data_in_stack(symStack, dataCopy);
            if (!data)
            {
                semantic_define_variable(symStack, dataCopy, true);
                data = find_data_in_stack(symStack, dataCopy);
            }

            op = create_operand_from_variable(data->unique_name, true);
            break;
        }
        default:
            return NULL;
    }
    free(dataCopy);
    return op;
}

static tDataType get_data_type_from_token(tToken token, tSymTableStack *symStack)
{
    if (!token)
        return TYPE_UNDEF;

    switch (token->type)
    {
        case T_INTEGER:
        case T_FLOAT:
            return TYPE_NUM;
        case T_STRING:
            return TYPE_STRING;
        case T_KW_NULL_VALUE:
            return TYPE_NULL;
        case T_ID:
        case T_GLOBAL_ID:
        {
            tSymbolData *data = find_data_in_stack(symStack, token->data);
            if (data)
            {
                return data->dataType;
            }
            return TYPE_UNDEF;
        }
        default:
            return TYPE_UNDEF;
    }
}

static tSymbol get_precedence_type(tToken token, FILE *file)
{
    if (!token)
        return E_DOLLAR;

    switch (token->type)
    {
        case T_MUL:
        case T_DIV:
            return E_MUL_DIV;

        case T_ADD:
        case T_SUB:
            return E_PLUS_MINUS;

        case T_LT:
        case T_GT:
        case T_LTE:
        case T_GTE:
            return E_REL;

        case T_EQL:
        case T_NEQ:
            return E_EQ_NEQ;

        case T_KW_IS:
            return E_IS;

        case T_KW_NUM:
        case T_KW_STRING:
        case T_KW_NULL_TYPE:
            return E_TYPE;

        case T_LEFT_PAREN:
            return E_LPAREN;

        case T_RIGHT_PAREN:
            return E_RPAREN;

        case T_INTEGER:
        case T_FLOAT:
        case T_STRING:
        case T_KW_NULL_VALUE:
            return E_LITERAL;

        case T_GLOBAL_ID:
            return E_ID;
        case T_ID:
        {
            tToken nextToken = peek_token(file);
            if (nextToken && nextToken->type == T_LEFT_PAREN)
            {
                return E_FUNC;
            }
            return E_ID;
        }

        case T_KW_IFJ:
            return E_FUNC;

        case T_EOF:
            return E_DOLLAR;

        default:
            return E_DOLLAR; // fallback
    }
}

static void expr_push(tExprStack *stack, tSymbol sym, bool isTerminal)
{
    tExprStackNode *node = (tExprStackNode *)safeMalloc(sizeof(tExprStackNode));
    node->symbol = sym;
    node->is_terminal = isTerminal;
    node->next = stack->top;
    node->dataType = TYPE_UNDEF;
    node->value = NULL;
    stack->top = node;
}

static void expr_pop(tExprStack *stack)
{
    if (stack->top == NULL)
        return;
    tExprStackNode *tmp = stack->top;
    stack->top = tmp->next;
    free(tmp);
}

static tExprStackNode *expr_top_terminal(tExprStack *stack)
{
    tExprStackNode *curr = stack->top;
    while (curr != NULL)
    {
        if (curr->is_terminal)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

static void expr_pop_until_marker(tExprStack *stack)
{
    while (stack->top != NULL && !(stack->top->is_terminal && stack->top->symbol == E_DOLLAR))
    {
        expr_pop(stack);
    }
}

static int is_token_expr_end(tToken *token)
{
    // Expression end tokens: ), ,, EOL, }, EOF
    if (token == NULL)
        return 1;
    switch ((*token)->type)
    {
        case T_RIGHT_PAREN:
        case T_COMMA:
        case T_EOL:
        case T_RIGHT_BRACE:
        case T_EOF:
            return 1;
        default:
            return 0;
    }
}

static int reduce_expr(tExprStack *stack)
{
    tExprStackNode *n1 = stack->top;

    if (n1 == NULL)
        return 0;

    // E -> i
    if (n1->is_terminal && (n1->symbol == E_ID || n1->symbol == E_LITERAL || n1->symbol == E_FUNC))
    {
        tDataType type = n1->dataType;
        n1->is_terminal = false;
        if (n1->value)
        {
            free(n1->value);
            n1->value = NULL;
        }
        n1->dataType = type;
        return 1;
    }

    tExprStackNode *n2 = n1->next;
    tExprStackNode *n3 = n2 ? n2->next : NULL;

    // E -> (E)
    if (n1 && n2 && n3)
    {
        if (n1->is_terminal && n1->symbol == E_RPAREN && !n2->is_terminal && n3->is_terminal &&
            n3->symbol == E_LPAREN)
        {
            tDataType type = n2->dataType;
            expr_pop(stack);
            expr_pop(stack);
            expr_pop(stack);
            expr_push(stack, n2->symbol, false);
            stack->top->dataType = type;
            return 1;
        }
    }

    // E -> - E (unary minus)
    if (n1 && n2 && n3 && !n1->is_terminal && n2->is_terminal && n3->is_terminal)
    {
        if (n2->symbol == E_PLUS_MINUS && strcmp(n2->value, "-") == 0)
        {
            if (n1->symbol == E_LITERAL && n3->symbol == E_LITERAL)
            {
                if (n1->dataType != TYPE_NUM || n3->dataType != TYPE_NUM)
                {
                    fprintf(stderr,
                            "[SEMANTIC] Error: Unary minus applied to non-numeric literal.\n");
                    exit(TYPE_COMPATIBILITY_ERROR);
                }
            }

            expr_pop(stack);
            if (n2->value)
                free(n2->value);
            expr_pop(stack);

            Operand *op1 = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
            emit(OP_DEFVAR, op1, NULL, NULL, &threeACcode);
            emit(OP_POPS, op1, NULL, NULL, &threeACcode);

            Operand *typeOp1 =
                create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
            emit(OP_DEFVAR, typeOp1, NULL, NULL, &threeACcode);
            emit(OP_TYPE, typeOp1, op1, NULL, &threeACcode);

            Operand *op1OkLabel = create_operand_from_label(threeAC_create_label(&threeACcode));
            emit(OP_JUMPIFNEQ, op1OkLabel, typeOp1, create_operand_from_constant_string("int"),
                 &threeACcode);
            emit(OP_INT2FLOAT, op1, op1, NULL, &threeACcode);
            emit(OP_LABEL, op1OkLabel, NULL, NULL, &threeACcode);

            // Push 0.0 and op1, then subtract
            Operand *zeroFloat = create_operand_from_constant_float(0.0);
            emit(OP_PUSHS, zeroFloat, NULL, NULL, &threeACcode);
            emit(OP_PUSHS, op1, NULL, NULL, &threeACcode);
            emit(OP_SUBS, NULL, NULL, NULL, &threeACcode);

            expr_push(stack, n3->symbol, false);
            stack->top->dataType = TYPE_NUM;
            return 1;
        }
    }

    // E -> E is TYPE
    if (n1 && n2 && n3)
    {
        if (n1->is_terminal && n1->symbol == E_TYPE && n2->is_terminal && n2->symbol == E_IS &&
            !n3->is_terminal)
        {

            char *typeStr = n1->value;
            expr_pop(stack);
            expr_pop(stack);
            expr_pop(stack);

            Operand *exprVal =
                create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
            emit(OP_DEFVAR, exprVal, NULL, NULL, &threeACcode);
            emit(OP_POPS, exprVal, NULL, NULL, &threeACcode);

            Operand *typeVal =
                create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
            emit(OP_DEFVAR, typeVal, NULL, NULL, &threeACcode);
            emit(OP_TYPE, typeVal, exprVal, NULL, &threeACcode);

            Operand *result =
                create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
            emit(OP_DEFVAR, result, NULL, NULL, &threeACcode);

            if (strcmp(typeStr, "Num") == 0)
            {
                Operand *isIntLabel = create_operand_from_label(threeAC_create_label(&threeACcode));
                Operand *endLabel = create_operand_from_label(threeAC_create_label(&threeACcode));

                emit(OP_JUMPIFEQ, isIntLabel, typeVal, create_operand_from_constant_string("int"),
                     &threeACcode);

                emit(OP_EQ, result, typeVal, create_operand_from_constant_string("float"),
                     &threeACcode);
                emit(OP_JUMP, endLabel, NULL, NULL, &threeACcode);

                emit(OP_LABEL, isIntLabel, NULL, NULL, &threeACcode);
                emit(OP_MOVE, result, create_operand_from_constant_bool(true), NULL, &threeACcode);

                emit(OP_LABEL, endLabel, NULL, NULL, &threeACcode);
            }
            else if (strcmp(typeStr, "Null") == 0)
            {
                emit(OP_EQ, result, typeVal, create_operand_from_constant_string("nil"),
                     &threeACcode);
            }
            else if (strcmp(typeStr, "String") == 0)
            {
                emit(OP_EQ, result, typeVal, create_operand_from_constant_string("string"),
                     &threeACcode);
            }
            else
            {
                emit(OP_MOVE, result, create_operand_from_constant_bool(false), NULL, &threeACcode);
            }

            emit(OP_PUSHS, result, NULL, NULL, &threeACcode);

            if (typeStr)
                free(typeStr);

            expr_push(stack, E_ID, false);
            stack->top->dataType = TYPE_UNDEF;
            return 1;
        }
    }

    // E -> E op E
    if (n1 && n2 && n3)
    {
        if (!n1->is_terminal && n2->is_terminal && !n3->is_terminal)
        {
            tDataType resultType = TYPE_UNDEF;

            // Static semantic analysis of literal types
            if (n1->symbol == E_LITERAL && n3->symbol == E_LITERAL)
            {
                resultType =
                    semantic_check_literal_operation(n2->value, n1->dataType, n3->dataType);
            }

            tSymbol op = n2->symbol;
            if (op == E_MUL_DIV || op == E_PLUS_MINUS || op == E_REL || op == E_EQ_NEQ)
            {

                if (strcmp(n2->value, "+") == 0)
                {
                    generate_add_op(&threeACcode);
                }
                else if (strcmp(n2->value, "*") == 0)
                {
                    generate_mult_op(&threeACcode);
                }
                else if (strcmp(n2->value, "-") == 0 || strcmp(n2->value, "/") == 0)
                {
                    generate_numeric_op(&threeACcode, n2->value);
                    resultType = TYPE_NUM;
                }
                else
                {
                    generate_relational_op(&threeACcode, n2->value);
                }

                expr_pop(stack);
                if (n2->value)
                    free(n2->value);
                expr_pop(stack);
                expr_pop(stack);

                if (n1->symbol == E_ID || n3->symbol == E_ID)
                {
                    expr_push(stack, E_ID, false);
                }
                else
                {
                    expr_push(stack, E_LITERAL, false);
                }

                stack->top->dataType = resultType;
                return 1;
            }
        }
    }

    return 0;
}

tDataType parse_expression(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    tExprStack exprStack = {NULL};
    expr_push(&exprStack, E_DOLLAR, true);

    tToken lookahead = *currentToken;
    int done = 0;

    while (!done)
    {
        tExprStackNode *topTerminal = expr_top_terminal(&exprStack);
        if (topTerminal == NULL)
        {
            exit(SYNTAX_ERROR);
        }

        tSymbol stackSym = topTerminal->symbol;
        tSymbol lookSym = get_precedence_type(lookahead, file);

        tPrec prec = precedence_table[stackSym][lookSym];

        if (prec == PREC_LESS || prec == PREC_EQUAL)
        {
            expr_push(&exprStack, lookSym, true);

            if (lookSym == E_ID || lookSym == E_LITERAL)
            {
                Operand *op = create_operand_from_token(lookahead, stack);
                emit(OP_PUSHS, op, NULL, NULL, &threeACcode);
                exprStack.top->dataType = get_data_type_from_token(lookahead, stack);
            }
            else if (lookSym == E_FUNC)
            {
                tDataType returnType = TYPE_UNDEF;
                if (lookahead->type == T_KW_IFJ)
                {
                    returnType = parse_ifj_call(file, &lookahead, stack, false);
                }
                else
                {
                    parse_function_call(file, &lookahead, stack, false);
                }
                exprStack.top->dataType = returnType;
            }
            else if (lookSym == E_TYPE)
            {
                char *typeKeyword = NULL;
                if (lookahead->type == T_KW_NUM)
                {
                    typeKeyword = "Num";
                }
                else if (lookahead->type == T_KW_STRING)
                {
                    typeKeyword = "String";
                }
                else if (lookahead->type == T_KW_NULL_TYPE)
                {
                    typeKeyword = "Null";
                }

                if (typeKeyword)
                {
                    exprStack.top->value = safeMalloc(strlen(typeKeyword) + 1);
                    strcpy(exprStack.top->value, typeKeyword);
                }
                else
                {
                    exprStack.top->value = NULL;
                }
            }
            else if (lookSym == E_PLUS_MINUS || lookSym == E_MUL_DIV || lookSym == E_REL ||
                     lookSym == E_EQ_NEQ)
            {
                if (lookahead->type == T_ADD)
                {
                    exprStack.top->value = safeMalloc(2);
                    strcpy(exprStack.top->value, "+");
                }
                else if (lookahead->type == T_SUB)
                {
                    exprStack.top->value = safeMalloc(2);
                    strcpy(exprStack.top->value, "-");
                }
                else if (lookahead->type == T_MUL)
                {
                    exprStack.top->value = safeMalloc(2);
                    strcpy(exprStack.top->value, "*");
                }
                else if (lookahead->type == T_DIV)
                {
                    exprStack.top->value = safeMalloc(2);
                    strcpy(exprStack.top->value, "/");
                }
                else if (lookahead->type == T_EQL)
                {
                    exprStack.top->value = safeMalloc(3);
                    strcpy(exprStack.top->value, "==");
                }
                else if (lookahead->type == T_NEQ)
                {
                    exprStack.top->value = safeMalloc(3);
                    strcpy(exprStack.top->value, "!=");
                }
                else if (lookahead->type == T_LT)
                {
                    exprStack.top->value = safeMalloc(2);
                    strcpy(exprStack.top->value, "<");
                }
                else if (lookahead->type == T_LTE)
                {
                    exprStack.top->value = safeMalloc(3);
                    strcpy(exprStack.top->value, "<=");
                }
                else if (lookahead->type == T_GT)
                {
                    exprStack.top->value = safeMalloc(2);
                    strcpy(exprStack.top->value, ">");
                }
                else if (lookahead->type == T_GTE)
                {
                    exprStack.top->value = safeMalloc(3);
                    strcpy(exprStack.top->value, ">=");
                }
            }

            if (lookahead->type == T_EOF)
            {
                break;
            }

            get_next_token(file, &lookahead);
        }
        else if (prec == PREC_GREATER)
        {
            if (!reduce_expr(&exprStack))
            {
                printf("Reduction failed\n");
                exit(SYNTAX_ERROR);
            }
        }
        else
        {
            printf("Unexpected token: %d\n", lookahead->type);
            printf("Unexpected token: %d:%d\n", lookahead->linePos, lookahead->colPos);
            exit(SYNTAX_ERROR);
        }

        tExprStackNode *n1 = exprStack.top;
        tExprStackNode *n2 = n1 ? n1->next : NULL;

        if (n1 && n2)
        {
            if (!n1->is_terminal && n2->is_terminal && n2->symbol == E_DOLLAR)
            {
                if (is_token_expr_end(&lookahead))
                {
                    done = 1;
                }
            }
        }
    }

    tDataType resultType = TYPE_UNDEF;

    if (exprStack.top && !exprStack.top->is_terminal)
    {
        resultType = exprStack.top->dataType;
        if (threeACcode.returnUsed == true)
        {
            Operand *retvalVar = create_operand_from_variable("%retval", false);
            emit(OP_POPS, retvalVar, NULL, NULL, &threeACcode);
            emit(OP_RETURN, NULL, NULL, NULL, &threeACcode);
            threeACcode.returnUsed = false;
        }
        threeACcode.expressionResult = NULL;
    }

    while (exprStack.top != NULL)
    {
        if (exprStack.top->value)
        {
            free(exprStack.top->value);
        }
        expr_pop(&exprStack);
    }

    *currentToken = lookahead;
    return resultType;
}
