#include "expr_parser.h"
#include "parser.h"
#include <stdio.h>
#include "3AC.h"
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
            char *inner_value = n2->value;
            expr_pop(stack);
            expr_pop(stack);
            expr_pop(stack);
            expr_push(stack, E_ID, false);
            stack->top->value = inner_value;
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
            if (op == E_MUL_DIV || op == E_PLUS_MINUS)
            {
                char *leftVal = n3->value;
                char *rightVal = n1->value;
                char *temp = threeAC_create_temp(&threeACcode);
        
        
                OperationType opType;
                if (strcmp(n2->value, "+") == 0) opType = OPP_ADD;
                else if (strcmp(n2->value, "-") == 0) opType = OPP_SUB;
                else if (strcmp(n2->value, "*") == 0) opType = OPP_MUL;
                else if (strcmp(n2->value, "/") == 0) opType = OPP_DIV;
                else {
                    exit(INTERNAL_ERROR);
                }

             
                emit(opType, leftVal, rightVal, temp, &threeACcode);
        
             
                expr_pop(stack); 
                expr_pop(stack);  
                expr_pop(stack);  
                expr_push(stack, E_ID, false);
                stack->top->value = temp;  
                return 1;
            }
        }
    }

    // E -> E is TYPE
    if (n1 && n2 && n3)
    {
        if (n1->is_terminal && n1->symbol == E_TYPE &&
            n2->is_terminal && n2->symbol == E_IS &&
            !n3->is_terminal)
        {
            expr_pop(stack);
            expr_pop(stack);
            expr_pop(stack);
            expr_push(stack, E_ID, false);
            return 1;
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
                 expr_stack.top->value = safeMalloc(strlen(lookahead->data) + 1);
                 strcpy(expr_stack.top->value, lookahead->data);
            }
            else if (look_sym == E_PLUS_MINUS || look_sym == E_MUL_DIV)
            {
                 char opChar;
                 if (lookahead->type == T_ADD) opChar = '+';
                 else if (lookahead->type == T_SUB) opChar = '-';
                 else if (lookahead->type == T_MUL) opChar = '*';
                 else if (lookahead->type == T_DIV) opChar = '/';
                
     
                 expr_stack.top->value = safeMalloc(2);
                 expr_stack.top->value[0] = opChar;
                 expr_stack.top->value[1] = '\0';
            }


    
            if (lookahead->type == T_EOF)
            {
                // EOF shifted, break if possible
                break;
            }

            get_next_token(file, &lookahead);
        }
        else if (prec == PREC_GREATER)
        {
            if (!reduce_expr(&expr_stack))
            {
                exit(SYNTAX_ERROR);
            }
        }
        else
        {
            exit(SYNTAX_ERROR);
        }

        // Check if top stack is $ E and lookahead is expression end token
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

    if (expr_stack.top && !expr_stack.top->is_terminal && expr_stack.top->value && expr_stack.top->value != NULL) {
       threeACcode.expression_result = expr_stack.top->value;
       threeACcode.temp_counter = 0;
       if (threeACcode.return_used == true) { 
          emit(OP_RETURN, NULL,NULL, threeACcode.expression_result, &threeACcode);
          threeACcode.return_used = false;
       } 
    }

    while (expr_stack.top != NULL)
    {
        expr_pop(&expr_stack);
    }

    *currentToken = lookahead;
    return 0;
}
