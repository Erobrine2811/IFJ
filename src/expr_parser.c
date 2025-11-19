#include "expr_parser.h"
#include "parser.h"
#include <stdio.h>
#include "3AC.h"
#include "semantic.h"

// Helper to create an Operand from a token
static Operand* create_operand_from_token(tToken token, tSymTableStack *sym_stack) {
    if (!token) return NULL;

    Operand *op = safeMalloc(sizeof(Operand));
    char *data_copy = NULL;
    if (token->data) {
        data_copy = safeMalloc(strlen(token->data) + 1);
        strcpy(data_copy, token->data);
    } else if (token->type == T_KW_NULL_VALUE) {
        op->type = OPP_CONST_NIL;
        return op;
    } else {
        free(op);
        return NULL;
    }

    switch(token->type) {
        case T_INTEGER:
            op->type = OPP_CONST_INT;
            op->value.intval = atoi(data_copy);
            free(data_copy);
            break;
        case T_FLOAT:
            op->type = OPP_CONST_FLOAT;
            op->value.floatval = atof(data_copy);
            free(data_copy);
            break;
        case T_STRING:
        {
            op->type = OPP_CONST_STRING;
            int len = strlen(data_copy);
            if (len < 2 || data_copy[0] != '"' || data_copy[len-1] != '"') {
                op->value.strval = data_copy;
                break;
            }

            char* unescaped_str = safeMalloc(len);
            int j = 0;
            for (int i = 1; i < len - 1; i++) {
                if (data_copy[i] == '\\') {
                    i++;
                    if (i >= len - 1) { break; }
                    switch (data_copy[i]) {
                        case 'n': unescaped_str[j++] = '\n'; break;
                        case 'r': unescaped_str[j++] = '\r'; break;
                        case 't': unescaped_str[j++] = '\t'; break;
                        case '"': unescaped_str[j++] = '"'; break;
                        case '\\': unescaped_str[j++] = '\\'; break;
                        case 'x':
                            if (i + 2 < len - 1 && isxdigit(data_copy[i+1]) && isxdigit(data_copy[i+2])) {
                                char hex[3] = {data_copy[i+1], data_copy[i+2], '\0'};
                                unescaped_str[j++] = (char)strtol(hex, NULL, 16);
                                i += 2;
                            } else {
                                unescaped_str[j++] = 'x';
                            }
                            break;
                        default:
                            unescaped_str[j++] = '\\';
                            unescaped_str[j++] = data_copy[i];
                            break;
                    }
                } else {
                    unescaped_str[j++] = data_copy[i];
                }
            }
            unescaped_str[j] = '\0';
            op->value.strval = safeRealloc(unescaped_str, j + 1);
            free(data_copy);
            break;
        }
        case T_KW_NULL_VALUE:
            op->type = OPP_CONST_NIL;
            free(data_copy);
            break;
        case T_ID:
        {
            tSymbolData *data = find_data_in_stack(sym_stack, data_copy);
            if (!data) {
                fprintf(stderr, "[SEMANTIC] Error: variable '%s' not found\n", data_copy);
                exit(OTHER_SEMANTIC_ERROR);
            }
            op->type = OPP_VAR;
            op->value.varname = safeMalloc(strlen(data->unique_name) + 1);
            strcpy(op->value.varname, data->unique_name);
            free(data_copy);
            break;
        }
        case T_GLOBAL_ID:
        {
            tSymbolData *data = find_data_in_stack(sym_stack, data_copy);
            if (!data) {
                semantic_define_variable(sym_stack, data_copy, true);
                data = find_data_in_stack(sym_stack, data_copy);
            }
            op->type = OPP_GLOBAL;
            op->value.varname = safeMalloc(strlen(data->unique_name) + 1);
            strcpy(op->value.varname, data->unique_name);
            free(data_copy);
            break;
        }
        default:
            free(op);
            free(data_copy);
            return NULL;
    }
    return op;
}

static tDataType get_data_type_from_token(tToken token, tSymTableStack *sym_stack) {
    if (!token) return TYPE_UNDEF;

    switch(token->type) {
        case T_INTEGER:
            return TYPE_INT;
        case T_FLOAT:
            return TYPE_FLOAT;
        case T_STRING:
            return TYPE_STRING;
        case T_KW_NULL_VALUE:
            return TYPE_NULL;
        case T_ID:
        case T_GLOBAL_ID:
        {
            tSymbolData *data = find_data_in_stack(sym_stack, token->data);
            if (data) {
                return data->dataType;
            }
            return TYPE_UNDEF;
        }
        default:
            return TYPE_UNDEF;
    }
}

static tPrec precedence_table[10][10] = {
/*                MUL_DIV  PLUS_MINUS  REL     EQ_NEQ   IS     TYPE    LPAREN  RPAREN  ID     DOLLAR */
/* MUL_DIV */    { '>',     '>',        '>',    '>',     '>',   'E',     '<',    '>',    '<',   '>'  },
/* PLUS_MINUS */ { '<',     '>',        '>',    '>',     '>',   'E',     '<',    '>',    '<',   '>'  },
/* REL */        { '<',     '<',        '>',    '>',     '>',   'E',     '<',    '>',    '<',   '>'  },
/* EQ_NEQ */     { '<',     '<',        '<',    '>',     '<',   'E',     '<',    '>',    '<',   '>'  },
/* IS */         { '<',     '<',        '<',    '<',     'E',   '<',     '<',    '>',    '<',   '>'  },
/* TYPE */       { '>',     '>',        '>',    '>',     '>',   '>',     'E',    '>',    'E',   '>'  },
/* LPAREN */     { '<',     '<',        '<',    '<',     '<',   '<',     '<',    '=',    '<',   'E'  },
/* RPAREN */     { '>',     '>',        '>',    '>',     '>',   '>',     'E',    '>',    'E',   '>'  },
/* ID */         { '>',     '>',        '>',    '>',     '>',   'E',     'E',    '>',    'E',   '>'  },
/* DOLLAR */     { '<',     '<',        '<',    '<',     '<',   '<',     '<',    'E',    '<',   'E'  },
};

static tSymbol get_precedence_type(tType type)
{
    switch (type)
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

        case T_ID:
        case T_GLOBAL_ID:
        case T_INTEGER:
        case T_FLOAT:
        case T_STRING:
        case T_KW_NULL_VALUE:
            return E_ID;

        case T_EOF:
            return E_DOLLAR;

        default:
            return E_DOLLAR; // fallback
    }
}

static void expr_push(tExprStack *stack, tSymbol sym, bool is_terminal)
{
    tExprStackNode *node = (tExprStackNode *)safeMalloc(sizeof(tExprStackNode));
    node->symbol = sym;
    node->is_terminal = is_terminal;
    node->next = stack->top;
    node->dataType = TYPE_UNDEF;
    node->value = NULL;
    stack->top = node;
}

static void expr_pop(tExprStack *stack)
{
    if (stack->top == NULL) return;
    tExprStackNode *tmp = stack->top;
    stack->top = tmp->next;
    free(tmp);
}

static tExprStackNode *expr_top_terminal(tExprStack *stack)
{
    tExprStackNode *curr = stack->top;
    while (curr != NULL)
    {
        if (curr->is_terminal) return curr;
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
    if (token == NULL) return 1;
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
    if (n1 == NULL) return 0;

    // E -> i
    if (n1->is_terminal && n1->symbol == E_ID)
    {
        tDataType type = n1->dataType;
        n1->is_terminal = false;
        if (n1->value) {
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
        if (n1->is_terminal && n1->symbol == E_RPAREN &&
           !n2->is_terminal &&
           n3->is_terminal && n3->symbol == E_LPAREN)
        {
            tDataType type = n2->dataType;
            expr_pop(stack);
            expr_pop(stack);
            expr_pop(stack);
            expr_push(stack, E_ID, false);
            stack->top->dataType = type;
            return 1;
        }
    }

    // E -> E op E
    if (n1 && n2 && n3)
    {
        if (!n1->is_terminal &&
           n2->is_terminal &&
           !n3->is_terminal)
        {
            tSymbol op = n2->symbol;
            if (op == E_MUL_DIV || op == E_PLUS_MINUS || op == E_REL || op == E_EQ_NEQ)
            {
                tDataType result_type = TYPE_UNDEF;
                if (op == E_MUL_DIV || op == E_PLUS_MINUS) {
                    result_type = semantic_check_operation(n2->value, n3->dataType, n1->dataType);
                }

                if (op == E_PLUS_MINUS && strcmp(n2->value, "+") == 0 && result_type == TYPE_STRING) {
                    Operand* res = safeMalloc(sizeof(Operand));
                    res->type = OPP_TEMP;
                    res->value.varname = threeAC_create_temp(&threeACcode);
                    emit(OP_DEFVAR, res, NULL, NULL, &threeACcode);

                    Operand* op2 = safeMalloc(sizeof(Operand));
                    op2->type = OPP_TEMP;
                    op2->value.varname = threeAC_create_temp(&threeACcode);
                    emit(OP_DEFVAR, op2, NULL, NULL, &threeACcode);
                    emit(OP_POPS, op2, NULL, NULL, &threeACcode);

                    Operand* op1 = safeMalloc(sizeof(Operand));
                    op1->type = OPP_TEMP;
                    op1->value.varname = threeAC_create_temp(&threeACcode);
                    emit(OP_DEFVAR, op1, NULL, NULL, &threeACcode);
                    emit(OP_POPS, op1, NULL, NULL, &threeACcode);

                    emit(OP_CONCAT, res, op1, op2, &threeACcode);

                    emit(OP_PUSHS, res, NULL, NULL, &threeACcode);
                } else if (op == E_MUL_DIV && strcmp(n2->value, "*") == 0 && result_type == TYPE_STRING) {
                    Operand* str_op = safeMalloc(sizeof(Operand));
                    str_op->type = OPP_TEMP;
                    str_op->value.varname = threeAC_create_temp(&threeACcode);
                    emit(OP_DEFVAR, str_op, NULL, NULL, &threeACcode);

                    Operand* num_op = safeMalloc(sizeof(Operand));
                    num_op->type = OPP_TEMP;
                    num_op->value.varname = threeAC_create_temp(&threeACcode);
                    emit(OP_DEFVAR, num_op, NULL, NULL, &threeACcode);

                    Operand* result_str = safeMalloc(sizeof(Operand));
                    result_str->type = OPP_TEMP;
                    result_str->value.varname = threeAC_create_temp(&threeACcode);
                    emit(OP_DEFVAR, result_str, NULL, NULL, &threeACcode);

                    emit(OP_POPS, num_op, NULL, NULL, &threeACcode);
                    emit(OP_POPS, str_op, NULL, NULL, &threeACcode);
                    emit(OP_MOVE, result_str, create_operand_from_constant_string(""), NULL, &threeACcode);
                
                    Operand *startWhileLabel = safeMalloc(sizeof(Operand));
                    startWhileLabel->type = OPP_LABEL;
                    startWhileLabel->value.label = threeAC_create_label(&threeACcode);
                    emit(OP_LABEL, startWhileLabel, NULL, NULL, &threeACcode);

                    emit(OP_PUSHS, num_op, NULL, NULL, &threeACcode);
                    emit(OP_PUSHS, create_operand_from_constant_int(0), NULL, NULL, &threeACcode);
                    emit(OP_GTS, NULL, NULL, NULL, &threeACcode);
                    emit(OP_PUSHS, create_operand_from_constant_bool(true), NULL, NULL, &threeACcode);

                    Operand *endWhileLabel = safeMalloc(sizeof(Operand));
                    endWhileLabel->type = OPP_LABEL;
                    endWhileLabel->value.label = threeAC_create_label(&threeACcode);

                    emit(OP_JUMPIFNEQS, NULL, NULL, endWhileLabel, &threeACcode);
                    emit(OP_PUSHS, num_op, NULL, NULL, &threeACcode);
                    emit(OP_PUSHS, create_operand_from_constant_int(1), NULL, NULL, &threeACcode);
                    emit(OP_SUBS, NULL, NULL, NULL, &threeACcode);
                    emit(OP_POPS, num_op, NULL, NULL, &threeACcode);

                    // Concatenate
                    emit(OP_CONCAT, result_str, result_str, str_op, &threeACcode);

                    emit(OP_JUMP, NULL, NULL, startWhileLabel, &threeACcode);
                    emit(OP_LABEL, endWhileLabel, NULL, NULL, &threeACcode);

                    emit(OP_PUSHS, result_str, NULL, NULL, &threeACcode);

                } else {
                    // Existing logic for stack-based operations
                    OperationType opType;
                    bool use_not = false;

                    if (strcmp(n2->value, "+") == 0) opType = OP_ADDS;
                    else if (strcmp(n2->value, "-") == 0) opType = OP_SUBS;
                    else if (strcmp(n2->value, "*") == 0) opType = OP_MULS;
                    else if (strcmp(n2->value, "/") == 0) {
                        if (n3->dataType == TYPE_INT && n1->dataType == TYPE_INT) { // int / int
                            // convert both to float
                            emit(OP_INT2FLOATS, NULL, NULL, NULL, &threeACcode); // convert right op (top of stack)
                            Operand* temp = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                            emit(OP_DEFVAR, temp, NULL, NULL, &threeACcode);
                            emit(OP_POPS, temp, NULL, NULL, &threeACcode); // pop converted right op
                            emit(OP_INT2FLOATS, NULL, NULL, NULL, &threeACcode); // convert left op
                            emit(OP_PUSHS, temp, NULL, NULL, &threeACcode); // push converted right op back
                        } else if (n3->dataType == TYPE_INT && (n1->dataType == TYPE_FLOAT || n1->dataType == TYPE_NUM)) { // int / float
                            Operand* temp = create_operand_from_variable(threeAC_create_temp(&threeACcode), false);
                            emit(OP_DEFVAR, temp, NULL, NULL, &threeACcode);
                            emit(OP_POPS, temp, NULL, NULL, &threeACcode); // pop float
                            emit(OP_INT2FLOATS, NULL, NULL, NULL, &threeACcode); // convert int
                            emit(OP_PUSHS, temp, NULL, NULL, &threeACcode); // push float back
                        } else if ((n3->dataType == TYPE_FLOAT || n3->dataType == TYPE_NUM) && n1->dataType == TYPE_INT) { // float / int
                            emit(OP_INT2FLOATS, NULL, NULL, NULL, &threeACcode);
                        }
                        // if both are float, do nothing
                        opType = OP_DIVS;
                    }
                    else if (strcmp(n2->value, "<") == 0) opType = OP_LTS;
                    else if (strcmp(n2->value, ">") == 0) opType = OP_GTS;
                    else if (strcmp(n2->value, "==") == 0) opType = OP_EQS;
                    else if (strcmp(n2->value, "!=") == 0) { opType = OP_EQS; use_not = true; }
                    else if (strcmp(n2->value, "<=") == 0) { opType = OP_GTS; use_not = true; }
                    else if (strcmp(n2->value, ">=") == 0) { opType = OP_LTS; use_not = true; }
                    else {
                        // Unknown operator
                        return 0;
                    }
                
                    emit(opType, NULL, NULL, NULL, &threeACcode);
                    if (use_not) {
                        emit(OP_NOTS, NULL, NULL, NULL, &threeACcode);
                    }
                }
             
                expr_pop(stack); 
                if (n2->value) free(n2->value);
                expr_pop(stack);  
                expr_pop(stack);  
                expr_push(stack, E_ID, false);
                stack->top->dataType = result_type;
                return 1;
            }
        }
    }

    return 0;
}

tDataType parse_expression(FILE *file, tToken *currentToken, tSymTableStack *stack)
{
    tExprStack expr_stack = { NULL };
    expr_push(&expr_stack, E_DOLLAR, true);

    tToken lookahead = *currentToken;
    int done = 0;

    while (!done)
    {
        tExprStackNode *top_terminal = expr_top_terminal(&expr_stack);
        if (top_terminal == NULL)
        {
            exit(SYNTAX_ERROR);
        }

        tSymbol stack_sym = top_terminal->symbol;
        tSymbol look_sym = get_precedence_type(lookahead->type);

        tPrec prec = precedence_table[stack_sym][look_sym];


        if (prec == PREC_LESS || prec == PREC_EQUAL) {
            expr_push(&expr_stack, look_sym, true);

            if (look_sym == E_ID) 
            {
                Operand *op = create_operand_from_token(lookahead, stack);
                if (op) {
                    emit(OP_PUSHS, op, NULL, NULL, &threeACcode);
                }
                expr_stack.top->dataType = get_data_type_from_token(lookahead, stack);
            }
            else if (look_sym == E_TYPE)
            {
                expr_stack.top->value = NULL;
            }
            else if (look_sym == E_PLUS_MINUS || look_sym == E_MUL_DIV || look_sym == E_REL || look_sym == E_EQ_NEQ)
            {
                if (lookahead->type == T_ADD) { 
                    expr_stack.top->value = safeMalloc(2);
                    strcpy(expr_stack.top->value, "+");
                }
                else if (lookahead->type == T_SUB) 
                { 
                    expr_stack.top->value = safeMalloc(2);
                    strcpy(expr_stack.top->value, "-");
                }
                else if (lookahead->type == T_MUL) 
                { 
                    expr_stack.top->value = safeMalloc(2);
                    strcpy(expr_stack.top->value, "*");
                }
                else if (lookahead->type == T_DIV) 
                {
                    expr_stack.top->value = safeMalloc(2);
                    strcpy(expr_stack.top->value, "/");
                } else if (lookahead->type == T_EQL) { 
                    expr_stack.top->value = safeMalloc(3);
                    strcpy(expr_stack.top->value, "==");
                } else if (lookahead->type == T_NEQ) { 
                    expr_stack.top->value = safeMalloc(3);
                    strcpy(expr_stack.top->value, "!=");
                } else if (lookahead->type == T_LT) { 
                    expr_stack.top->value = safeMalloc(2);
                    strcpy(expr_stack.top->value, "<");
                } else if (lookahead->type == T_LTE) { 
                    expr_stack.top->value = safeMalloc(3);
                    strcpy(expr_stack.top->value, "<=");
                } else if (lookahead->type == T_GT) { 
                    expr_stack.top->value = safeMalloc(2);
                    strcpy(expr_stack.top->value, ">");
                } else if (lookahead->type == T_GTE) { 
                    expr_stack.top->value = safeMalloc(3);
                    strcpy(expr_stack.top->value, ">=");
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
            if (!reduce_expr(&expr_stack))
            {
                printf("Reduction failed\n");
                exit(SYNTAX_ERROR);
            }
        }
        else
        {
            exit(SYNTAX_ERROR);
        }

        tExprStackNode *n1 = expr_stack.top;
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

    tDataType result_type = TYPE_UNDEF;
    if (expr_stack.top && !expr_stack.top->is_terminal) {
       result_type = expr_stack.top->dataType;
       if (threeACcode.return_used == true) {
          Operand* retval_var = create_operand_from_variable("%retval", false);
          emit(OP_POPS, retval_var, NULL, NULL, &threeACcode);
          emit(OP_RETURN, NULL, NULL, NULL, &threeACcode);
          threeACcode.return_used = false;
       }
       threeACcode.expression_result = NULL;
    }

    while (expr_stack.top != NULL)
    {
        if (expr_stack.top->value) {
            free(expr_stack.top->value);
        }
        expr_pop(&expr_stack);
    }

    *currentToken = lookahead;
    return result_type;
}
