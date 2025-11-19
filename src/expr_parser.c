#include "expr_parser.h"
#include "parser.h"
#include <stdio.h>
#include "3AC.h"

// Helper to create an Operand from a token
static Operand* create_operand_from_token(tToken token) {
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
            op->type = OPP_CONST_STRING;
            op->value.strval = data_copy;
            break;
        case T_KW_NULL_VALUE:
            op->type = OPP_CONST_NIL;
            free(data_copy);
            break;
        case T_ID:
            op->type = OPP_VAR;
            op->value.varname = data_copy;
            break;
        case T_GLOBAL_ID:
            op->type = OPP_GLOBAL;
            op->value.varname = data_copy;
            break;
        default:
            free(op);
            free(data_copy);
            return NULL;
    }
    return op;
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
        n1->is_terminal = false;
        if (n1->value) {
            free(n1->value);
            n1->value = NULL;
        }
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
            expr_pop(stack);
            expr_pop(stack);
            expr_pop(stack);
            expr_push(stack, E_ID, false);
            stack->top->value = NULL;
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
                OperationType opType;
                bool use_not = false;

                if (strcmp(n2->value, "+") == 0) opType = OP_ADDS;
                else if (strcmp(n2->value, "-") == 0) opType = OP_SUBS;
                else if (strcmp(n2->value, "*") == 0) opType = OP_MULS;
                else if (strcmp(n2->value, "/") == 0) opType = OP_DIVS;
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
             
                expr_pop(stack); 
                if (n2->value) free(n2->value);
                expr_pop(stack);  
                expr_pop(stack);  
                expr_push(stack, E_ID, false);
                stack->top->value = NULL;
                return 1;
            }
        }
    }

    return 0;
}

int parse_expression(FILE *file, tToken *currentToken, tSymTableStack *stack)
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
                Operand *op = create_operand_from_token(lookahead);
                if (op) {
                    emit(OP_PUSHS, op, NULL, NULL, &threeACcode);
                }
                expr_stack.top->value = NULL;
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

    if (expr_stack.top && !expr_stack.top->is_terminal) {
       if (threeACcode.return_used == true) {
          Operand* ret_val = safeMalloc(sizeof(Operand));
          ret_val->type = OPP_TEMP;
          ret_val->value.varname = threeAC_create_temp(&threeACcode);
          emit(OP_DEFVAR, ret_val, NULL, NULL, &threeACcode);
          emit(OP_POPS, ret_val, NULL, NULL, &threeACcode);
          emit(OP_RETURN, ret_val, NULL, NULL, &threeACcode);
          threeACcode.return_used = false;
       }
       threeACcode.expression_result = NULL;
    }
    threeACcode.temp_counter = 0;

    while (expr_stack.top != NULL)
    {
        if (expr_stack.top->value) {
            free(expr_stack.top->value);
        }
        expr_pop(&expr_stack);
    }

    *currentToken = lookahead;
    return 0;
}
